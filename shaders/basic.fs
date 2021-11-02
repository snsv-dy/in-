#version 460 core

precision mediump float;

out vec4 FragColor;
in vec3 position;
in vec3 color;

float elevationFunc(vec2 pos);

vec3 getNorm(vec2 pos) {
	const float step = 0.1;
	float R = elevationFunc(pos + vec2(step, 0.0));
	float L = elevationFunc(pos - vec2(step, 0.0));
	float T = elevationFunc(pos + vec2(0.0, step));
	float B = elevationFunc(pos - vec2(0.0, step));

	return normalize(vec3(2.0 * (R - L), 2.0 * (B - T), -1.0));
}

float elevationFunc(vec2 pos) {
	pos *= 4.0;
	float rd = 0.5;
	return sin(pos.x) * cos(pos.y) * rd + 0.5;
}


void main() {

	vec3 sunPos = vec3(0.0, 2.0, -2.0);

	vec3 samples[4];
	samples[0] = vec3(1.0, 1.0, 0.0);
	samples[1] = vec3(0.0, 0.1, 0.0);
	samples[2] = vec3(0.7, 0.7, 1.0);
	samples[3] = vec3(1.0, 0.0, 0.0);

	// vec3 col = vec3(0.0);
	// if (position.x > 0.0) {
	// 	col += position.x * samples[0];
	// } else {
	// 	col += -position.x * samples[1];
	// }
	
	// if (position.y > 0.0) {
	// 	col += position.y * samples[2];
	// } else {
	// 	col += -position.y * samples[3];
	// }
	// col /= 2.0;
	// float mid = 0.1;
	// vec3 col = vec3(1.0, 1.0, 1.0);
	// if (position.y > mid) {
	// 	col = samples[0];
	// } else {
	// 	col = samples[2];
	// }

	vec3 col = color;

	float diff = dot(getNorm(position.xz), normalize(sunPos));
	col *= diff;
	// col = vec3(1.0, abs(position.y), abs(position.z));

	FragColor = vec4(col, 1.0);
}