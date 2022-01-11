#version 460 core

precision mediump float;

out vec4 FragColor;
in vec3 position;
in vec3 color;
in vec3 norm;
in vec2 texPos;

uniform sampler2D heightmapTexture;
uniform sampler2D colorTexture;

uniform bool colorData;
uniform vec3 sunPos;

float elevationFunc(vec2 pos);

vec3 getNorm(vec2 pos) {
	// const float step = 0.01;
	const float step = 10.0 / 512.0;
	// float R = elevationFunc(pos + vec2(step, 0.0));
	// float L = elevationFunc(pos - vec2(step, 0.0));
	// float T = elevationFunc(pos + vec2(0.0, step));
	// float B = elevationFunc(pos - vec2(0.0, step));

	float R = (textureOffset(heightmapTexture, pos, ivec2(1, 0)).r + 
				textureOffset(heightmapTexture, pos, ivec2(1, 1)).r + 
				textureOffset(heightmapTexture, pos, ivec2(1, -1)).r) / 3.0 ;

	float L = (textureOffset(heightmapTexture, pos, ivec2(-1, 0)).r + 
				textureOffset(heightmapTexture, pos, ivec2(-1, 1)).r + 
				textureOffset(heightmapTexture, pos, ivec2(-1, -1)).r) / 3.0 ;

	float T = (textureOffset(heightmapTexture, pos, ivec2(0, 1)).r + 
				textureOffset(heightmapTexture, pos, ivec2(1, 1)).r + 
				textureOffset(heightmapTexture, pos, ivec2(-1, 1)).r) / 3.0 ;

	float B = (textureOffset(heightmapTexture, pos, ivec2(0, -1)).r + 
				textureOffset(heightmapTexture, pos, ivec2(1, -1)).r + 
				textureOffset(heightmapTexture, pos, ivec2(-1, -1)).r) / 3.0 ;

	// return normalize(vec3(2.0 * (R - L), 1.0, 2.0 * (B - T)));
	vec3 u = vec3(0.0, T - B, step);
	vec3 v = vec3(step, R - L, 0.0);
	return normalize(cross(u, v));
	//return texture(heightmapTexture, pos / 2.0).r;
}

float elevationFunc(vec2 pos) {
	//return 0.0;
	return texture(heightmapTexture, pos).r;
	//pos *= 1.0;
	//float rd = 0.5;
	//return pos.x * pos.x;
	//return sin(pos.x) * cos(pos.y) * rd;
}


void main() {

	// sunPos = vec3(0.0, 10.0, 0.0);

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

	vec3 col = colorData ? texture(colorTexture, texPos).rgb : color;

	//float diff = max(dot(getNorm(texPos), normalize(sunPos)), 0.0);
	vec3 normal = norm;//getNorm(texPos);
	// normal = vec3(normal.x, -normal.z, normal.y);
	vec3 lightDir = normalize(sunPos - vec3(texPos.x, texture(heightmapTexture, texPos).r, texPos.y));
	float diff = max(dot(normal, lightDir), 0.0);
	const float ambient = 0.1;
	col *= max(diff, ambient);
	// col = vec3(1.0, abs(position.y), abs(position.z));

	FragColor = vec4(col, 1.0);
}