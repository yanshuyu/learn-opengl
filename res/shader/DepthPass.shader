#shader vertex
#version 450 core
#include "Transform.glsl"

layout(location = 0) in vec3 a_pos;
layout(location = 4) in ivec4 a_bones;
layout(location = 5) in vec4 a_weights;

uniform mat4 u_VPMat;

invariant gl_Position;

void main() {
	gl_Position = u_VPMat * u_Transform(vec4(a_pos, 1.f), a_bones, a_weights);
}

