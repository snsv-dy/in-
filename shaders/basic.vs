#version 460 core

in vec3 pos;
out vec3 position;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

float elevationFunc(vec2 pos) {
	pos *= 4.0;
	float rd = 0.3;
	return sin(pos.x) * cos(pos.y) * rd;
}

void main() {
	position = pos;
	position.y = elevationFunc(pos.xz);
	gl_Position = projection * view * model * vec4(position, 1.0);
}