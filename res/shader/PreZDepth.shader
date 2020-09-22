#shader vertex
#version 450 core

layout(location = 0) in vec3 a_pos;

layout(location = 0) uniform mat4 u_MVP;

invariant gl_Position;

void main() {
	gl_Position = u_MVP * vec4(a_pos, 1.0);
}

