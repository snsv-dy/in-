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
	float water_value = imageLoad(water, pixel_coords).x;

	float valueL = 
		imageLoad(img_output, pixel_coords + ivec2(-1, 0)).x
		+ imageLoad(water, pixel_coords + ivec2(-1, 0)).x;
	float deltaL = value + water_value;
	deltaL -= valueL;

	float valueR = 
		imageLoad(img_output, pixel_coords + ivec2(1, 0)).x
		+ imageLoad(water, pixel_coords + ivec2(1, 0)).x;
	float deltaR = value + water_value;
	deltaR -= valueR;

	float valueT = 
		imageLoad(img_output, pixel_coords + ivec2(0, 1)).x
		+ imageLoad(water, pixel_coords + ivec2(0, 1)).x;
	float deltaT = value + water_value;
	deltaT -= valueT;

	float valueB = 
		imageLoad(img_output, pixel_coords + ivec2(0, -1)).x
		+ imageLoad(water, pixel_coords + ivec2(0, -1)).x;
	float deltaB = value + water_value;
	deltaB -= valueB;

	return vec4(deltaL, deltaR, deltaT, deltaB);
	// return vec4(0.0, deltaR, 0.0, 0.0);
}

void main() {
	const float dt = 0.01;
	const float pipe_area = 1.1;
	const float pipe_length = 1.1;
	const float cell_area = 1.0;
	const float g = 9.8; 

	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	ivec2 dims = imageSize(img_output);
	float x = float(pixel_coords.x) / dims.x;
	float y = float(pixel_coords.y) / dims.y;
	
	float value = imageLoad(img_output, pixel_coords).x;
	vec4 water_value = imageLoad(water, pixel_coords);

	if (stage == 0) {
		vec4 deltas = computeDeltas(pixel_coords, dims);
		vec4 prevFlow = imageLoad(flow, pixel_coords);
		vec4 vFlow = vec4(0.0);
		vFlow.x = max(0.0, prevFlow.x + dt * pipe_area * g * (deltas.x) / pipe_length);
		vFlow.y = max(0.0, prevFlow.y + dt * pipe_area * g * (deltas.y) / pipe_length);
		vFlow.z = max(0.0, prevFlow.z + dt * pipe_area * g * (deltas.z) / pipe_length);
		vFlow.w = max(0.0, prevFlow.w + dt * pipe_area * g * (deltas.w) / pipe_length);
		
		// vec4 vFlow = prevFlow + dt * pipe_area * g * (deltas) / pipe_length;
		float K = min(1.0, water_value.x * cell_area * cell_area / (vFlow.x + vFlow.y + vFlow.z + vFlow.w) * dt);
		// vFlow *= K;
		// vFlow.r = 1.0;
		// vFlow.g = 0.0;
		// vFlow.b = 0.0;
		// vFlow.a = 0.0;
		imageStore(flow, pixel_coords, vFlow);
		// imageStore(water, pixel_coords, vec4(K));
		memoryBarrier();
	} else if (stage == 1) {
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

		float next_water = water_value.x + dV/(cell_area * cell_area);

		vec4 water_save = vec4(next_water, 0.0, 0.0, 1.0);
		// vec4 water_save = vec4(0.0, 1.0, 0.0, 1.0);

		imageStore(water, pixel_coords, water_save);
		memoryBarrier();
	}

	// vec4 pixel = vec4(vec3(deltaL), 1.0);
	// vec4 pixel = deltas;

	// imageStore(water, pixel_coords, pixel);

	// float value2 = flowIn;//imageLoad(water, pixel_coords).x;
	// vec4 pixel2 = vec4(vec3(value2), 1.0);

	// imageStore(img_output, pixel_coords, pixel2);
}