#shader vertex
#version 450 core

#define MAX_NUM_BONE 156

layout(location = 0) in vec3 a_pos;
layout(location = 4) in ivec4 a_bones;
layout(location = 5) in vec4 a_weights;


uniform mat4 u_ModelMat;
uniform mat4 u_SkinPose[MAX_NUM_BONE];

subroutine vec4 TransformType(vec4 pos, ivec4 bones, vec4 weights);
subroutine uniform TransformType u_Transform;

subroutine(TransformType)
vec4 staticMesh(vec4 pos, ivec4 bones, vec4 weights) {
	return u_ModelMat * pos;
}

subroutine(TransformType)
vec4 skinMesh(vec4 pos, ivec4 bones, vec4 weights) {
	mat4 skinMat = u_SkinPose[bones.x] * weights.x
		+ u_SkinPose[bones.y] * weights.y
		+ u_SkinPose[bones.z] * weights.z
		+ u_SkinPose[bones.w] * weights.w;

	return u_ModelMat * skinMat * pos;
}

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

