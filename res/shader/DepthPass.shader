#shader vertex
#version 450 core

#define MAX_NUM_BONE 156

layout(location = 0) in vec3 a_pos;
layout(location = 4) in ivec4 a_bones;
layout(location = 5) in vec4 a_weights;

uniform mat4 u_ModelMat;
uniform mat4 u_VPMat;
uniform mat4 u_SkinPose[MAX_NUM_BONE];

subroutine vec4 TransformType(vec3 pos, ivec4 bones, vec4 weights);
subroutine uniform TransformType u_Transform;

subroutine (TransformType)
vec4 staticMesh(vec3 pos, ivec4 bones, vec4 weights) {
	return u_ModelMat * vec4(pos, 1.f);
}

subroutine (TransformType)
vec4 skinMesh(vec3 pos, ivec4 bones, vec4 weights) {
	mat4 skinMat = u_SkinPose[bones.x] * weights.x
					+ u_SkinPose[bones.y] * weights.y
					+ u_SkinPose[bones.z] * weights.z
					+ u_SkinPose[bones.w] * weights.w;
	
	return u_ModelMat * skinMat * vec4(pos, 1.f);
}

invariant gl_Position;

void main() {
	gl_Position = u_VPMat * u_Transform(a_pos, a_bones, a_weights);
}

