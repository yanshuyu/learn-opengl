#shader vertex
#version 450 core

layout(location = 0) in vec3 a_pos;

layout(location = 0) uniform mat4 u_ModelMat;
layout(location = 1) uniform mat4 u_VPMat;

invariant gl_Position;

void main() {
	gl_Position = u_VPMat * u_ModelMat * vec4(a_pos, 1.0);
}

