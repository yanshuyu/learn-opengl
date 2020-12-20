#shader vertex
#version 450 core
#include "Transform.glsl"

layout(location = 0) in vec3 a_pos;
layout(location = 4) in ivec4 a_bones;
layout(location = 5) in vec4 a_weights;



void main() {
	gl_Position = u_Transform(vec4(a_pos, 1.f), a_bones, a_weights);
}


///////////////////////////////////


#shader geometry
#version 450 core

#define MAXNUMCASCADE 4

layout(triangles) in;
layout(triangle_strip, max_vertices = 12) out;

uniform mat4 u_lightVP[MAXNUMCASCADE];
uniform int u_numCascade;

void main() {
	for (int cascade = 0; cascade < u_numCascade; cascade++) {
		gl_Layer = cascade;
		for (int i = 0; i < 3; i++) {
			gl_Position = u_lightVP[cascade] * gl_in[i].gl_Position;
			EmitVertex();
		}
		EndPrimitive();
	}
}


///////////////////////////////////


#shader fragment
#version 450 core

void main() {

}

