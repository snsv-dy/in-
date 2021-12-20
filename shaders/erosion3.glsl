#version 430
layout (local_size_x = 1) in;
layout (rgba32f, binding = 0) volatile uniform image2D img_output;

struct Particle {
	vec2 Position;
	vec2 Direction;
	float speed;
	float water;
	float sediment;
	int alive;
};

layout(std430, binding = 1) volatile buffer SSBO {
	Particle pt[];
};

vec3 gradient(vec2 pos) {
	// int x = (int)posx;
	// int y = (int)posy;
	int mapw = 512;
	// if (x > 0 && x < 511 && y > 0 && y < 511) {
	ivec2 ipos = ivec2(pos);
	
	float NW = imageLoad(img_output, ipos).x;
	float NE = imageLoad(img_output, ipos + ivec2(1, 0)).x;
	float SW = imageLoad(img_output, ipos + ivec2(0, 1)).x;
	float SE = imageLoad(img_output, ipos + ivec2(1, 1)).x;

	float lx = pos.x - floor(pos.x);
	float ly = pos.y - floor(pos.y);

	float gradx = (NE - NW) * (1 - ly) + (SE - SW) * ly;
	float grady = (SW - NW) * (1 - lx) + (SE - NE) * lx;

	float height = NW * (1 - lx) * (1 - ly) + NE * lx * (1 - ly) + SW * (1 - lx) * ly + SE * lx * ly;

	return vec3(gradx, grady, height);


		// return true;
	// }
}

void depositSediment(vec2 pos, float amount) {
	float cellx = pos.x - floor(pos.x);
	float celly = pos.y - floor(pos.y);
	
	ivec2 ipos = ivec2(pos);
	if (ipos.x < 510 && ipos.y < 510) {
		vec4 TL_source = imageLoad(img_output, ipos);
		TL_source.x += amount * (1.0 - cellx) * (1.0 - celly);
		imageStore(img_output, ipos, TL_source);

		vec4 TR_source = imageLoad(img_output, ipos + ivec2(1, 0));
		TR_source.x += amount * cellx * (1.0 - celly);
		imageStore(img_output, ipos + ivec2(1, 0), TR_source);

		vec4 BL_source = imageLoad(img_output, ipos + ivec2(0, 1));
		BL_source.x += amount * (1.0 - cellx) * celly;
		imageStore(img_output, ipos + ivec2(1, 1), BL_source);

		vec4 BR_source = imageLoad(img_output, ipos + ivec2(1, 1));
		BR_source.x += amount * cellx * celly;
		imageStore(img_output, ipos + ivec2(1, 1), BR_source);
	}
}

void erodeTerrain(vec2 pos, float erosionAmount) {
	float cellx = pos.x - floor(pos.x);
	float celly = pos.y - floor(pos.y);
	
	ivec2 ipos = ivec2(pos);
	if (ipos.x < 510 && ipos.y < 510) {
		vec4 TL_source = imageLoad(img_output, ipos);
		TL_source.x -= erosionAmount * (1.0 - cellx) * (1.0 - celly);
		imageStore(img_output, ipos, TL_source);

		vec4 TR_source = imageLoad(img_output, ipos + ivec2(1, 0));
		TR_source.x -= erosionAmount * cellx * (1.0 - celly);
		imageStore(img_output, ipos + ivec2(1, 0), TR_source);

		vec4 BL_source = imageLoad(img_output, ipos + ivec2(0, 1));
		BL_source.x -= erosionAmount * (1.0 - cellx) * celly;
		imageStore(img_output, ipos + ivec2(1, 1), BL_source);

		vec4 BR_source = imageLoad(img_output, ipos + ivec2(1, 1));
		BR_source.x -= erosionAmount * cellx * celly;
		imageStore(img_output, ipos + ivec2(1, 1), BR_source);
	}
}

void main() {
	const float capacityParam = 4.0f; 
	const float minCapacity = 0.01f;
	const float depositionRate = 0.3f;
	const float erosionRate = 0.3f;
	const float evaporationRate = 0.02f;
	const float gravity = 4.0f;
	const float inertia = 0.05f;
	
	ivec2 dims = imageSize(img_output);

	uint index = gl_GlobalInvocationID.x;
	if (pt[index].alive == 1) {
		for (int i = 0; i < 30; i++) {
			vec2 pos = pt[index].Position;
			ivec2 ipos = ivec2(pos);
			if (ipos.x < 0 || ipos.y >= dims.x || ipos.y < 0 || ipos.y >= dims.y) {
				pt[index].alive = 0;
			} else {
				vec3 grad = gradient(pos);

				vec2 dir = pt[index].Direction;
				dir = dir * inertia - grad.xy * (1 - inertia);

				float dirlen = max(0.01f, sqrt(dir.x * dir.x + dir.y * dir.y));
				dir /= dirlen;

				pos += dir;

				float oldHeight = grad.z;
				float newHeight = gradient(pos).z;
				float dHeight = newHeight - oldHeight;
				
				float speed = pt[index].speed;
				float water = pt[index].water;
				float capacity = max(-dHeight * speed * water * capacityParam, minCapacity);

				float sediment = pt[index].sediment;

				if (sediment > capacity || dHeight > 0.0) {
					float depositedAmount = (dHeight > 0.0) ? min(dHeight, sediment) : (sediment - capacity) * depositionRate;
					sediment -= depositedAmount;

					depositSediment(pos, depositedAmount);
				} else {
					float erosionAmount = min((capacity - sediment) * erosionRate, -dHeight);
					sediment += erosionAmount;

					erodeTerrain(pos, erosionAmount);
				}

				pt[index].speed = sqrt(max(0.0, speed * speed + dHeight * gravity));
				pt[index].water = pt[index].water * (1.0 - evaporationRate);
				pt[index].Position = pos;
				pt[index].Direction = dir;
				pt[index].sediment = sediment;
				// pt[index].Position += pt[index].Direction;
				// imageStore(img_output, ipos, vec4(vec3(grad.z + 0.1, 0.0, 0.0), 1.0));
			}
			memoryBarrier();
		}
	}
}

	// uint index = gl_GlobalInvocationID.x;
	// Position[gl_GlobalInvocationID.x] = 1.0;
	// ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	// float col = float(pixel_coords.x);
	// float nx = float(gl_NumWorkGroups.x);