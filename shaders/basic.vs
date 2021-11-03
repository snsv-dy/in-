#version 460 core

in vec3 pos;
out vec3 position;
out vec2 texPos;
out vec3 color;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform sampler2D heightmap;

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
	position.y = texture(heightmap, texPos).r;


	float minv = -0.5;
	float maxv = 0.5;
	float range = maxv - minv;
	int steps = 4;
	float stepv = range / (steps - 1);

	if (position.y > minv + 0.0 * stepv && position.y < minv + 1.0 * stepv) {
		color = mix(samples[0], samples[1], remap(minv + 0.0 * stepv, minv + 1.0 * stepv, position.y));
	} else if (position.y >= minv + 1.0 * stepv && position.y < minv + 2.0 * stepv) {
		color = mix(samples[1], samples[2], remap(minv + 1.0 * stepv, minv + 2.0 * stepv, position.y));
	} else {
		color = mix(samples[2], samples[3], remap(minv + 2.0 * stepv, minv + 3.0 * stepv, position.y));
	}

	gl_Position = projection * view * model * vec4(position.xyz, 1.0);
}