#version 430
layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D img_output;
layout(rgba32f, binding = 2) uniform image2D water;
layout(rgba32f, binding = 3) uniform image2D flow;

uniform sampler2D heightmap_input;

vec4 computeDeltas(ivec2 pixel_coords, ivec2 dims) {
	float x = float(pixel_coords.x) / dims.x;
	float y = float(pixel_coords.y) / dims.y;

	float value = texture(heightmap_input, vec2(x, y)).x;

	float xl = float(pixel_coords.x - 1) / dims.x;
	float valueL = texture(heightmap_input, vec2(xl, y)).x;
	float deltaL = value;
	deltaL -= valueL;

	float xr = float(pixel_coords.x + 1) / dims.x;
	float valueR = texture(heightmap_input, vec2(xr, y)).x;
	float deltaR = value;
	deltaR -= valueR;

	float yt = float(pixel_coords.y + 1) / dims.y;
	float valueT = texture(heightmap_input, vec2(x, yt)).x;
	float deltaT = value;
	deltaT -= valueT;

	float yb = float(pixel_coords.y - 1) / dims.y;
	float valueB = texture(heightmap_input, vec2(x, yb)).x;
	float deltaB = value;
	deltaB -= valueB;

	return vec4(deltaL, deltaR, deltaT, deltaB);
}

void main() {
	const float dt = 2.0;
	const float pipe_area = 0.1;
	const float pipe_length = 0.1;
	const float cell_area = 1.0;
	const float g = 9.8; 

	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	ivec2 dims = imageSize(img_output);
	float x = float(pixel_coords.x) / dims.x;
	float y = float(pixel_coords.y) / dims.y;
	
	float value = texture(heightmap_input, vec2(x, y)).x;
	vec4 water_value = imageLoad(water, pixel_coords);

	vec4 deltas = computeDeltas(pixel_coords, dims);
	vec4 vFlow = max(vec4(0.0), imageLoad(flow, pixel_coords) + dt * pipe_area * g * (deltas) / pipe_length);
	float K = min(0.0, water_value.x * cell_area * cell_area / (vFlow.x + vFlow.y + vFlow.z + vFlow.w) * dt);
	vFlow *= K;
	imageStore(flow, pixel_coords, vFlow);

	float flowIn = 
		imageLoad(flow, pixel_coords + ivec2(1, 0)).x + 
		imageLoad(flow, pixel_coords + ivec2(-1, 0)).y + 
		imageLoad(flow, pixel_coords + ivec2(0, 1)).w + 
		imageLoad(flow, pixel_coords + ivec2(0, -1)).z;


	// vec4 pixel = vec4(vec3(deltaL), 1.0);
	vec4 pixel = deltas;

	// imageStore(water, pixel_coords, pixel);

	float value2 = flowIn;//imageLoad(water, pixel_coords).x;
	vec4 pixel2 = vec4(vec3(value2), 1.0);

	imageStore(img_output, pixel_coords, pixel2);
}