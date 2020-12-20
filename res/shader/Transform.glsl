
#define MAX_NUM_BONE 156

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