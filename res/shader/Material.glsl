
layout(std140) uniform MaterialBlock{
	vec4 albedo; // rgb(diffuse/emissive) a(opacity)
	vec4 specular; // rgb(specular) a(shinness)
	float emissive;
	float roughness;
	float metalness;
}ub_Mtl;

uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_SpecularMap; // phong material property
uniform sampler2D u_RoughnessMap; // PBR meterial property
uniform sampler2D u_MetallicMap;
uniform ivec4 u_HasANRMMap; // xyz for Phong material has albedo/normal/specular
							// xyzw for PBR material has albedo/normal/roughness/metallic


vec3 sRGB2RGB(in vec3 color) {
	return pow(color, vec3(2.2f));
}