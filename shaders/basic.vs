#version 460 core

in vec3 pos;
out vec3 position;
out vec2 texPos;
out vec3 color;
out vec3 norm;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform sampler2D heightmapTexture;

float elevationFunc(vec2 pos) {
	pos *= 1.0;
	float rd = 0.5;
	//return pos.x * pos.x;
	return sin(pos.x) * cos(pos.y) * rd;
}

float remap( float minval, float maxval, float curval )
{
    return ( curval - minval ) / ( maxval - minval );
} 

vec3 getNorm(vec2 pos) {
	const float step = 0.01;
	// const float step = 4.0 / 512.0;
	float R = texture(heightmapTexture, pos + vec2(step, 0.0)).r;
	float L = texture(heightmapTexture, pos - vec2(step, 0.0)).r;
	float T = texture(heightmapTexture, pos + vec2(0.0, step)).r;
	float B = texture(heightmapTexture, pos - vec2(0.0, step)).r;

	// float R = (textureOffset(heightmapTexture, pos, ivec2(1, 0)).r);
	//  + 
	// 			textureOffset(heightmapTexture, pos, ivec2(1, 1)).r + 
	// 			textureOffset(heightmapTexture, pos, ivec2(1, -1)).r) / 3.0 ;

	// float L = (textureOffset(heightmapTexture, pos, ivec2(-1, 0)).r);
	//  + 
	// 			textureOffset(heightmapTexture, pos, ivec2(-1, 1)).r + 
	// 			textureOffset(heightmapTexture, pos, ivec2(-1, -1)).r) / 3.0 ;

	// float T = (textureOffset(heightmapTexture, pos, ivec2(0, 1)).r);
	//  + 
	// 			textureOffset(heightmapTexture, pos, ivec2(1, 1)).r + 
	// 			textureOffset(heightmapTexture, pos, ivec2(-1, 1)).r) / 3.0 ;

	// float B = (textureOffset(heightmapTexture, pos, ivec2(0, -1)).r);
	//  + 
	// 			textureOffset(heightmapTexture, pos, ivec2(1, -1)).r + 
	// 			textureOffset(heightmapTexture, pos, ivec2(-1, -1)).r) / 3.0 ;

	return normalize(vec3(0.5 * (R - L), step * 2.0, 0.5 * (B - T)));
	// vec3 u = vec3(0.0, T - B, step);
	// vec3 v = vec3(step, R - L, 0.0);
	// return normalize(cross(u, v));
	//return texture(heightmapTexture, pos / 2.0).r;
}

void main() {
	vec3 samples[4];
	samples[0] = vec3(0.0, 0.0, 1.0);
	samples[1] = vec3(0.0, 1.0, 0.0);
	samples[2] = vec3(1.0, 1.0, 0.0);
	samples[3] = vec3(1.0, 0.0, 0.0);

	//samples[3] = vec3(0.0, 0.0, 1.0);
	//samples[2] = vec3(0.0, 1.0, 0.0);
	//samples[1] = vec3(1.0, 1.0, 0.0);
	//samples[0] = vec3(1.0, 0.0, 0.0);

	position = pos;
	texPos = position.xz;
	texPos.x += 1.0;
	texPos.x /= 2.0;
	texPos.y += 1.0;
	texPos.y /= 2.0;

	float el = elevationFunc(pos.xz);
	position.y = texture(heightmapTexture, texPos).r;

	norm = getNorm(texPos);

	float minv = 0.0;
	float maxv = 1.0;
	float range = maxv - minv;
	int steps = 4;
	float stepv = range / (steps - 1);

	if (position.y >= minv + 0.0 * stepv && position.y < minv + 1.0 * stepv) {
		color = mix(samples[0], samples[1], remap(minv + 0.0 * stepv, minv + 1.0 * stepv, position.y));
	} else if (position.y >= minv + 1.0 * stepv && position.y < minv + 2.0 * stepv) {
		color = mix(samples[1], samples[2], remap(minv + 1.0 * stepv, minv + 2.0 * stepv, position.y));
	} else {
		color = mix(samples[2], samples[3], remap(minv + 2.0 * stepv, minv + 3.0 * stepv, position.y));
	}

	gl_Position = projection * view * model * vec4(position.xyz, 1.0);
}