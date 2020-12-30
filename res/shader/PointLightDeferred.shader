#shader vertex
#version 450 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;

out vec2 f_uv;

void main() {
	gl_Position = vec4(a_pos, 1.f);
	f_uv = a_uv;
}


/////////////////////////////////////////////////////////////////////


#shader fragment
#version 450 core
#include "Phong.glsl"
#include "PBR.glsl"

const float diskRadius = 0.05f;

const vec3 sampleOffsetDirections[20] = {
	vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
	vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
	vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
	vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
	vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
};



in vec2 f_uv;
out vec4 frag_color;


uniform sampler2D u_PosW;
uniform sampler2D u_NormalW;
uniform sampler2D u_Albedo;
uniform sampler2D u_Specular;
uniform sampler2D u_EMR; // emissive/metallic/roughness
uniform sampler2D u_ShadeMode;

uniform float u_maxShininess;
uniform vec3 u_cameraPosW;

uniform samplerCube u_shadowMap;
uniform float u_shadowStrength;
uniform float u_shadowBias;

layout(std140) uniform LightBlock{
	vec4 u_lightPos; // (w for range)
	vec4 u_lightColor; //(a for intensity)
};

subroutine float ShadowAttenType(vec3 posW);
subroutine uniform ShadowAttenType u_shadowAtten;

subroutine(ShadowAttenType)
float noShadow(vec3 posW) {
	return 1.f;
}

subroutine(ShadowAttenType)
float hardShadow(vec3 posW) {
	vec3 l2v = posW - u_lightPos.xyz;
	float depth = length(l2v) / u_lightPos.w;
	if (depth > 1.f)
		return 1.f;

	float closestDepth = texture(u_shadowMap, l2v).r;
	return depth + u_shadowBias > closestDepth ? (1.f - u_shadowStrength) : 1.f;
}

subroutine(ShadowAttenType)
float softShadow(vec3 posW) {
	vec3 l2v = posW - u_lightPos.xyz;
	float depth = length(l2v) / u_lightPos.w;
	if (depth > 1.f)
		return 1.f;

	float shadow = 0.f;
	for (int i = 0; i < 20; i++) {
		float closestDepth = texture(u_shadowMap, l2v + sampleOffsetDirections[i] * diskRadius).r;
		shadow += depth + u_shadowBias > closestDepth ? 1.f : 0.f;
	}

	return mix(1.f - u_shadowStrength, 1.f, 1.f - shadow / 20.f);
}


void main() {
	vec3 P = texture(u_PosW, f_uv).xyz;
	
	vec3 l = u_lightPos.xyz - P;
	float distance = length(l);
	float rangeAtten = 1.f - smoothstep(0.f, u_lightPos.w, distance);

	vec3 I = u_lightColor.rgb * u_lightColor.a * rangeAtten;
	vec3 L = l / distance;
	vec3 N = (texture(u_NormalW, f_uv).xyz - 0.5) * 2;
	vec3 V = normalize(u_cameraPosW - P);
	vec3 A = texture(u_Albedo, f_uv).rgb;
	vec3 EMR = texture(u_EMR, f_uv).rgb;
	int Mode = texture(u_ShadeMode, f_uv).r;

	vec3 C = vec3(1.f, 0.f, 0.f);
	if (Mode == 1) {
		vec4 S = texture(u_Specular, f_uv);
		vec3 ks = S.rgb;
		float shinness = S.a * u_maxShininess;
		C = Phong(I, L, N, V, A, ks, shinness);
	}
	else if (Mode == 2) {
		C = PBR(I, L, N, V, A, EMR.g, EMR.b);
	}

	float shadowAtte = u_shadowAtten(P);

	frag_color = vec4(C * shadowAtte, 1.f);
}


