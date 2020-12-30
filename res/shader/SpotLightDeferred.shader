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

const int PCFCornelSize = 5;

in vec2 f_uv;
out vec4 frag_color;


uniform sampler2D u_PosW;
uniform sampler2D u_NormalW;
uniform sampler2D u_Albedo;
uniform sampler2D u_Specular;
uniform sampler2D u_EMR; // emissive/metallic/roughness
uniform sampler2D u_ShadeMode;

uniform float u_maxShininess;

uniform sampler2D u_shadowMap;
uniform bool u_hasShadowMap;
uniform float u_shadowStrength;
uniform	float u_shadowBias;

uniform	mat4 u_lightVP;
uniform vec3 u_cameraPosW;

layout(std140) uniform LightBlock{
	vec4 u_lightPos; // (wfor range)
	vec4 u_lightColor; //(a for intensity)
	vec3 u_toLight;
	vec2 u_angles;
};


subroutine float ShadowAttenType(vec3 P);
subroutine uniform ShadowAttenType u_shadowAtten;

subroutine(ShadowAttenType)
float noShadow(vec3 P) {
	return 1.f;
}

subroutine(ShadowAttenType)
float hardShadow(vec3 P) {
	vec4 posProj = u_lightVP * vec4(P, 1.f);
	vec3 posNDC = posProj.xyz / posProj.w;
	posNDC = posNDC * 0.5f + 0.5f;
	if (posNDC.z > 1.f)
		return 0.f;

	float depth = texture(u_shadowMap, posNDC.xy).r;
	bool inShaow = posNDC.z + u_shadowBias > depth;
	return inShaow ? 1.f - u_shadowStrength : 1.f;
}


subroutine(ShadowAttenType)
float softShadow(vec3 P) {
	vec4 posProj = u_lightVP * vec4(P, 1.f);
	vec3 posNDC = posProj.xyz / posProj.w;
	posNDC = posNDC * 0.5f + 0.5f;
	if (posNDC.z > 1.f)
		return 0.f;

	vec2 texelSize = 1.f / textureSize(u_shadowMap, 0);
	int halfCornelSize = PCFCornelSize / 2;
	int shadowArea = 0;
	for (int x = -halfCornelSize; x <= halfCornelSize; x++) {
		for (int y = -halfCornelSize; y <= halfCornelSize; y++) {
			float depth = texture(u_shadowMap, posNDC.xy + vec2(x, y) * texelSize).r;
			shadowArea += posNDC.z + u_shadowBias > depth ? 1 : 0;
		}
	}

	return mix(1.f - u_shadowStrength, 1.f, 1.f - shadowArea / float(PCFCornelSize * PCFCornelSize));
}



void main() {
	vec3 P = texture(u_PosW, f_uv).xyz;

	vec3 l = u_lightPos.xyz - P;
	float distance = length(l);
	vec3 L = l / distance;

	float rangeAtten = 1.f - smoothstep(0.f, u_lightPos.w, distance);
	float angleAtten = smoothstep(u_angles.x * 0.5f, u_angles.y * 0.5f, acos(dot(L, u_toLight)));

	vec3 I = u_lightColor.rgb * u_lightColor.a * rangeAtten * angleAtten;
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
	
	float shadowAtten = u_shadowAtten(P);

	frag_color = vec4(C * shadowAtten, 1.f);
}


