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
	vec3 pos_W;
	vec3 normal_W;
	vec3 tangent_W;
	vec2 uv;
} vs_out;


layout(location = 0) uniform mat4 u_VPMat;
layout(location = 1) uniform mat4 u_ModelMat;
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
	vs_out.pos_W = u_Transform(vec4(a_pos, 1.f), a_bones, a_weights).xyz;

	vs_out.normal_W = normalize(u_Transform(vec4(a_normal, 0.f), a_bones, a_weights).xyz);
	vs_out.tangent_W = normalize(u_Transform(vec4(a_tangent, 0.f), a_bones, a_weights).xyz);

	vs_out.uv = a_uv;
}


///////////////////////////////////////////////////////////



#shader fragment
#version 450 core

in VS_OUT{
	vec3 pos_W;
	vec3 normal_W;
	vec3 tangent_W;
	vec2 uv;
} fs_in;

layout(location = 2) uniform sampler2D u_diffuseMap;
layout(location = 3) uniform bool u_hasDiffuseMap;

layout(location = 4) uniform sampler2D u_normalMap;
layout(location = 5) uniform bool u_hasNormalMap;

layout(location = 6) uniform sampler2D u_specularMap;
layout(location = 7) uniform bool u_hasSpecularMap;

layout(location = 8) uniform sampler2D u_emissiveMap;
layout(location = 9) uniform bool u_hasEmissiveMap;


layout(std140) uniform MatrialBlock{
	vec4 u_diffuseFactor; //(a for alpha)
	vec4 u_specularFactor; // (a for shininess)
	vec3 u_emissiveColor; 
};


layout(location = 0) out vec4 o_posW;
layout(location = 1) out vec4 o_normalW;
layout(location = 2) out vec4 o_diffuse;
layout(location = 3) out vec4 o_specular;
layout(location = 4) out vec4 o_emissive;


vec3 sRGB2RGB(in vec3 color) {
	return color * color;
}

void main() {
	vec3 dTex = u_hasDiffuseMap ? sRGB2RGB(texture(u_diffuseMap, fs_in.uv).rgb) : vec3(1.f);
	vec3 sTex = u_hasSpecularMap ? sRGB2RGB(texture(u_specularMap, fs_in.uv).rgb) : vec3(1.f);
	vec3 eTex = u_hasEmissiveMap ? sRGB2RGB(texture(u_emissiveMap, fs_in.uv).rgb) : vec3(1.f);

	vec3 normalW = fs_in.normal_W;
	if (u_hasNormalMap) {
		vec3 normal = (texture(u_normalMap, fs_in.uv).xyz - 0.5) * 2;
		vec3 biTangent = normalize(cross(fs_in.normal_W, fs_in.tangent_W));
		normalW = normalize(mat3(fs_in.tangent_W, biTangent, fs_in.normal_W) * normal);
	}

	o_posW = vec4(fs_in.pos_W, 1.f);
	o_normalW = vec4(normalW * 0.5 + 0.5, 0.f);
	o_diffuse = vec4(dTex * u_diffuseFactor.rgb, u_diffuseFactor.a);
	o_specular = vec4(sTex * u_specularFactor.rgb, u_specularFactor.a);
	o_emissive = vec4(eTex * u_emissiveColor, 0.f);
}
