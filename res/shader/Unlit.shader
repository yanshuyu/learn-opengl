#shader vertex
#version 450 core

#define MAX_NUM_BONE 156

layout(location = 0) in vec3 a_pos;
layout(location = 3) in vec2 a_uv;
layout(location = 4) in ivec4 a_bones;
layout(location = 5) in vec4 a_weights;

out vec2 f_uv;

uniform mat4 u_VPMat;
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


invariant gl_Position;

void main() {
	gl_Position = u_VPMat * u_Transform(vec4(a_pos, 1.f), a_bones, a_weights);
	f_uv = a_uv;
}



///////////////////////////////////////////////////////////////////////////



#shader fragment
#version 450 core

in vec2 f_uv;

uniform sampler2D u_diffuseMap;
uniform bool u_hasDiffuseMap;
uniform vec4 u_diffuseColor;

layout(location = 0) out vec4 frag_color;


vec3 sRGB2RGB(in vec3 color) {
	return color * color;
}


void main() {
	vec3 texColor = u_hasDiffuseMap ? sRGB2RGB(texture(u_diffuseMap, f_uv).rgb) : vec3(1.f);
	frag_color = vec4(u_diffuseColor.rgb * texColor, 1.0);
}
