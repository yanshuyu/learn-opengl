#shader vertex
#version 450 core

#define MAX_NUM_BONE 156

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec3 a_tangent;
layout(location = 3) in vec2 a_uv;
layout(location = 4) in ivec4 a_bones;
layout(location = 5) in vec4 a_weights;


out VS_OUT{
	vec3 normal_W;
	vec3 tangent_W;
	vec2 uv;
} vs_out;


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


void main() {
	gl_Position = u_VPMat * u_Transform(vec4(a_pos, 1.f), a_bones, a_weights);

	vs_out.normal_W = normalize(u_Transform(vec4(a_normal, 0.f), a_bones, a_weights).xyz);
	vs_out.tangent_W = normalize(u_Transform(vec4(a_tangent, 0.f), a_bones, a_weights).xyz);
	vs_out.uv = a_uv;
}



///////////////////////////////////////////////



#shader fragment
#version 450 core


in VS_OUT{
	vec3 normal_W;
	vec3 tangent_W;
	vec2 uv;
}fs_in;

out vec4 frag_color;

uniform sampler2D u_DiffuseMap;
uniform bool u_HasDiffuseMap;

uniform sampler2D u_NormalMap;
uniform bool u_HasNormalMap;

uniform vec3 u_AmbientSky;
uniform vec3 u_AmbientGround;


vec3 calcAmibientLight() {
	vec3 normalW = fs_in.normal_W;
	if (u_HasNormalMap) {
		vec3 normal = (texture(u_NormalMap, fs_in.uv).xyz - 0.5) * 2;
		vec3 biTangent = normalize(cross(fs_in.normal_W, fs_in.tangent_W));
		normalW = normalize(mat3(fs_in.tangent_W, biTangent, fs_in.normal_W) * normal);
	}

	return mix(u_AmbientGround, u_AmbientSky, normalW.y * 0.5 + 0.5);
}


void main() {
	vec4 mainColor = u_HasDiffuseMap ? texture(u_DiffuseMap, fs_in.uv) : vec4(1.f);
	if (mainColor.a < 0.05f)
		discard;

	mainColor.rgb *= mainColor.rgb; // sRGB to RGb

	frag_color = vec4(calcAmibientLight() * mainColor.rgb, 1.f);
}

