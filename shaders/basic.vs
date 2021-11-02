#version 460 core

in vec3 pos;
out vec3 position;
out vec3 color;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

float elevationFunc(vec2 pos) {
	pos *= 4.0;
	float rd = 0.5;
	return sin(pos.x) * cos(pos.y) * rd + 0.5;
}

float remap( float minval, float maxval, float curval )
{
    return ( curval - minval ) / ( maxval - minval );
} 

void main() {
	vec3 samples[4];
	//samples[0] = vec3(0.0, 0.0, 1.0);
	//samples[1] = vec3(0.0, 1.0, 0.0);
	//samples[2] = vec3(1.0, 1.0, 0.0);
	//samples[3] = vec3(1.0, 0.0, 0.0);

	samples[3] = vec3(0.0, 0.0, 1.0);
	samples[2] = vec3(0.0, 1.0, 0.0);
	samples[1] = vec3(1.0, 1.0, 0.0);
	samples[0] = vec3(1.0, 0.0, 0.0);

	position = pos;
	position.y = elevationFunc(pos.xz);

	if (position.y > 0 && position.y < 0.3) {
		color = mix(samples[0], samples[1], remap(0.0, 0.3, position.y));
	} else if (position.y >= 0.3 && position.y < 0.6) {
		color = mix(samples[1], samples[2], remap(0.3, 0.6, position.y));
	} else {
		color = mix(samples[2], samples[3], remap(0.6, 1.0, position.y));
	}

	gl_Position = projection * view * model * vec4(position, 1.0);
}