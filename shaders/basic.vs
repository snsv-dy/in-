#version 300 es

in vec3 pos;
out vec3 position;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

float elevationFunc(vec2 pos) {
	pos *= 1.0;
	float rd = 0.01;
	return sin(pos.x) * cos(pos.y);
}

void main() {
	position = pos;
	position.y = elevationFunc(pos.xz);
	gl_Position = projection * view * model * vec4(position, 1.0);
}