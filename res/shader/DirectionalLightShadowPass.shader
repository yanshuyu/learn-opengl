#shader vertex
#version 450 core

layout(location = 0) in vec3 a_pos;

layout(location = 0) uniform mat4 u_ModelMat;


void main() {
	gl_Position = u_ModelMat * vec4(a_pos, 1.f);
}


///////////////////////////////////


#shader geometry
#version 450 core

#define MAXNUMCASCADE 4

layout(triangles) in;
layout(triangle_strip, max_vertices = 12) out;

layout(location = 1) uniform mat4 u_lightVP[MAXNUMCASCADE];
layout(location = 5) uniform int u_numCascade;

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

