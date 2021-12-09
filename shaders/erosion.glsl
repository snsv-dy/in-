#version 430
layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D img_output;
layout(rgba32f, binding = 2) uniform image2D water;
layout(rgba32f, binding = 3) uniform image2D flow;

uniform sampler2D heightmap_input;
uniform int stage;

// void main() {
// 	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
// 	ivec2 dims = imageSize(img_output);
// 	float x = float(pixel_coords.x) / dims.x;
// 	float y = float(pixel_coords.y) / dims.y;	

// 	for (int i = 0; i < 1; i++) {
// 		vec4 prev = imageLoad(img_output, pixel_coords + ivec2(0, 0));
// 		// memoryBarrier();

// 		float prev_value = prev.x;

// 		float value = prev_value + 0.01;

// 		vec4 pixel = vec4(vec3(value), 1.0);

// 		imageStore(img_output, pixel_coords, pixel);
// 		// memoryBarrier();
// 	}
// }

vec4 computeDeltas(ivec2 pixel_coords, ivec2 dims) {
	float x = float(pixel_coords.x) / dims.x;
	float y = float(pixel_coords.y) / dims.y;

	float value = imageLoad(img_output, pixel_coords).x;
	float water_value = imageLoad(img_output, pixel_coords).y;

	float valueL = 
		imageLoad(img_output, pixel_coords + ivec2(-1, 0)).x
		+ imageLoad(img_output, pixel_coords + ivec2(-1, 0)).y;
	float deltaL = value + water_value;
	deltaL -= valueL;

	float valueR = 
		imageLoad(img_output, pixel_coords + ivec2(1, 0)).x
		+ imageLoad(img_output, pixel_coords + ivec2(1, 0)).y;
	float deltaR = value + water_value;
	deltaR -= valueR;

	float valueT = 
		imageLoad(img_output, pixel_coords + ivec2(0, 1)).x
		+ imageLoad(img_output, pixel_coords + ivec2(0, 1)).y;
	float deltaT = value + water_value;
	deltaT -= valueT;

	float valueB = 
		imageLoad(img_output, pixel_coords + ivec2(0, -1)).x
		+ imageLoad(img_output, pixel_coords + ivec2(0, -1)).y;
	float deltaB = value + water_value;
	deltaB -= valueB;

	return vec4(deltaL, deltaR, deltaT, deltaB);
	// return vec4(0.0, deltaR, 0.0, 0.0);
}

vec3 calcNorm(ivec2 pixel_coords) {
	const int step = 1;
	float R = imageLoad(img_output, pixel_coords + ivec2(step, 0)).x;
	float L = imageLoad(img_output, pixel_coords + ivec2(-step, 0)).x;
	float T = imageLoad(img_output, pixel_coords + ivec2(0, step)).x;
	float B = imageLoad(img_output, pixel_coords + ivec2(0, -step)).x;
	return normalize(vec3(2.0 * (R - L), 2.0 * (B - T), -1.0));
}

void main() {
	const float dt = 0.02;
	const float pipe_area = 2.0;
	const float pipe_length = 3.0;
	const float cell_size = 10.0;
	const float g = 9.8; 
	const float Capacity_constant = 0.3;
	const float Dissolving_rate = 0.2;
	const float Deposition_rate = 0.05;
	const float Evaporation_constant = 0.015;
	const float Rain_rate = 0.12;

	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	ivec2 dims = imageSize(img_output);
	float x = float(pixel_coords.x) / dims.x;
	float y = float(pixel_coords.y) / dims.y;
	
	vec4 T1 = imageLoad(img_output, pixel_coords);
	vec4 Tw = imageLoad(water, pixel_coords);
	float value = T1.x;
	float water_value = T1.y;

	if (stage == 0) {
		// Update Flow
		vec4 deltas = computeDeltas(pixel_coords, dims);
		vec4 prevFlow = imageLoad(flow, pixel_coords);
		vec4 vFlow = vec4(0.0);
		vFlow.x = max(0.0, prevFlow.x + dt * pipe_area * g * (deltas.x) / pipe_length);
		vFlow.y = max(0.0, prevFlow.y + dt * pipe_area * g * (deltas.y) / pipe_length);
		vFlow.z = max(0.0, prevFlow.z + dt * pipe_area * g * (deltas.z) / pipe_length);
		vFlow.w = max(0.0, prevFlow.w + dt * pipe_area * g * (deltas.w) / pipe_length);
		
		// vec4 vFlow = prevFlow + dt * pipe_area * g * (deltas) / pipe_length;
		float K = min(1.0, water_value * cell_size * cell_size / (vFlow.x + vFlow.y + vFlow.z + vFlow.w) * dt);
		vFlow *= K;
		// vFlow.r = 1.0;
		// vFlow.g = 0.0;
		// vFlow.b = 0.0;
		// vFlow.a = 0.0;
		imageStore(flow, pixel_coords, vFlow);
		// imageStore(water, pixel_coords, vec4(K));
		memoryBarrier();
	} else if (stage == 1) {
		// Update water height
		float flowIn = 
			imageLoad(flow, pixel_coords + ivec2(1, 0)).x + 
			imageLoad(flow, pixel_coords + ivec2(-1, 0)).y + 
			imageLoad(flow, pixel_coords + ivec2(0, 1)).w + 
			imageLoad(flow, pixel_coords + ivec2(0, -1)).z;
		vec4 flow_value = imageLoad(flow, pixel_coords);
		float flowOut = 
			flow_value.x + 
			flow_value.y + 
			flow_value.w + 
			flow_value.z;
		
		float dV = (flowIn - flowOut);

		float next_water = water_value + dV/(cell_size * cell_size);
		float avg_water = (water_value + next_water) / 2.0;

		// vec4 water_save = vec4(value, next_water, T1.z, 1.0);
		// vec4 water_save = vec4(0.0, 1.0, 0.0, 1.0);
		T1.y = next_water;
		T1.w = 1.0;
		imageStore(img_output, pixel_coords, T1);
		Tw.z = avg_water;
		imageStore(water, pixel_coords, Tw);
		memoryBarrier();
	} else if (stage == 2) {
		// Calculate velocity
		// vec4 velocity = imageLoad(water, pixel_coords);
		float Wx = imageLoad(flow, pixel_coords + ivec2(-1, 0)).y -
			imageLoad(flow, pixel_coords).x + 
			imageLoad(flow, pixel_coords).y -
			imageLoad(flow, pixel_coords + ivec2(1, 0)).x;
		float vx = Wx / (Tw.z * cell_size * 2.0);
		
		float Wy = imageLoad(flow, pixel_coords + ivec2(0, 1)).w -
			imageLoad(flow, pixel_coords).z + 
			imageLoad(flow, pixel_coords).w -
			imageLoad(flow, pixel_coords + ivec2(0, -1)).z;
		float vy = Wy / (Tw.z * cell_size * 2.0);
		Tw.x = vx;
		Tw.y = vy;
		imageStore(water, pixel_coords, Tw);
		// imageStore(water, pixel_coords, vec4(0.0, 0.0, velocity.z, 1.0));
	} else if (stage == 3) {
		// Erosion and deposition stage
		const vec3 up = vec3(0.0, 1.0, 0.0);
		vec3 norm = calcNorm(pixel_coords);
		float ang = acos(dot(norm, up));

		vec4 velocity = imageLoad(water, pixel_coords);

		float capacity = Capacity_constant * sin(ang) * length(velocity.xy);
		float sediment = Tw.w;

		float next_terrain = T1.x;
		float next_sediment = sediment;
		if (capacity > sediment) {
			float capacity_minus_sediment = Dissolving_rate * (capacity - sediment);
			next_terrain -= capacity_minus_sediment;
			next_sediment += capacity_minus_sediment;
		} else {
			float sediment_minus_capacity = Deposition_rate * (sediment - capacity);
			next_terrain += sediment_minus_capacity;
			next_sediment -= sediment_minus_capacity;
		}

		T1.x = next_terrain;
		T1.z = next_sediment;
		imageStore(img_output, pixel_coords, T1);
	} else if (stage == 4) {
		// Sediment transportation stage
		// vec4 velocity = imageLoad(water, pixel_coords);

		float sediment = T1.z;
		// vec2 fpos = vec2(float(pixel_coords.x), float(pixel_coords.y));
		// vec2 newPos = vec2(fpos.x - dt * Tw.x, fpos.y - dt * Tw.y);
		// ivec2 new_ipos = ivec2(int(newPos.x), int(newPos.y));
		ivec2 new_ipos = ivec2(pixel_coords.x - dt * Tw.x, pixel_coords.y - dt * Tw.y);
		vec4 new_water = imageLoad(water, new_ipos);
		new_water.w = sediment;
		imageStore(water, new_ipos, new_water);
	} else if (stage == 5) {
		// Water evaporation
		T1.y *= (1 - Evaporation_constant * dt);
		imageStore(img_output, pixel_coords, T1);
	} else if (stage == 6) {
		T1.y += Rain_rate * dt;
		imageStore(img_output, pixel_coords, T1);
	}

	// vec4 pixel = vec4(vec3(deltaL), 1.0);
	// vec4 pixel = deltas;

	// imageStore(water, pixel_coords, pixel);

	// float value2 = flowIn;//imageLoad(water, pixel_coords).x;
	// vec4 pixel2 = vec4(vec3(value2), 1.0);

	// imageStore(img_output, pixel_coords, pixel2);
}