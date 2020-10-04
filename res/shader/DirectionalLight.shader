#shader vertex
#version 450 core


layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 4) in vec2 a_uv;


out vec3 pos_W;
out vec3 normal_W;
out vec2 f_uv;
out float z_Proj; // project space depth



layout(location = 0) uniform mat4 u_VPMat;
layout(location = 1) uniform mat4 u_ModelMat;

invariant gl_Position;

void main() {
	gl_Position = u_VPMat * u_ModelMat * vec4(a_pos, 1.f);
	pos_W = (u_ModelMat * vec4(a_pos, 1.f)).xyz;
	normal_W = (u_ModelMat * vec4(a_normal, 0.f)).xyz;
	f_uv = a_uv;
	z_Proj = gl_Position.z;
}


/////////////////////////////////////////////////////////////////////


#shader fragment
#version 450 core

#define MAXNUMCASCADE 4
#define PCFCornelSize 3

in vec3 pos_W;
in vec3 normal_W;
in vec2 f_uv;
in float z_Proj;


layout(location = 2) uniform sampler2D u_diffuseMap;
layout(location = 3) uniform bool u_hasDiffuseMap;

layout(location = 4) uniform sampler2D u_normalMap;
layout(location = 5) uniform bool u_hasNormalMap;

layout(location = 6) uniform sampler2D u_specularMap;
layout(location = 7) uniform bool u_hasSpecularMap;

layout(location = 8) uniform sampler2D u_emissiveMap;
layout(location = 9) uniform bool u_hasEmissiveMap;

layout(location = 10) uniform vec3 u_cameraPosW;

layout(location = 12) uniform sampler2DArray u_shadowMapArray;

layout(location = 13) uniform mat4 u_lightVP[MAXNUMCASCADE];
layout(location = 17) uniform float u_cascadesFarZ[MAXNUMCASCADE];
layout(location = 21) uniform int u_numCascade;

layout(location = 22) uniform float u_shadowStrength;
layout(location = 23) uniform float u_shadowBias;

layout(std140) uniform LightBlock {
	vec4 u_lightColor; //(a for intensity)
	vec3 u_toLight;
};


layout(std140) uniform MatrialBlock {
	vec4 u_diffuseFactor; //(a for alpha)
	vec4 u_specularFactor; // (a for shininess)
	vec3 u_emissiveColor; // (a for embient absord)
};


subroutine float ShadowAttenType(vec3 posW, vec3 normalW);
subroutine uniform ShadowAttenType u_shadowAtten;

vec3 sRGB2RGB(in vec3 color) {
	return color * color;
}

int calcCascadeIndex(in float projDepth) {
	int idx = 0;
	for (int i = 0; i < u_numCascade; i++) {
		if (projDepth <= u_cascadesFarZ[i]) {
			idx = i;
			break;
		}
	}
	return idx;
}

subroutine (ShadowAttenType)
float noShadow(vec3 posW, vec3 normalW) {
	return 1.f;
}

subroutine (ShadowAttenType)
float hardShadow(vec3 posW, vec3 normalW) {
	int cascadeIdx = calcCascadeIndex(z_Proj);
	vec4 posL = u_lightVP[cascadeIdx] * vec4(posW, 1.f); // convert to light space
	vec3 posProj = posL.xyz / posL.w;
	posProj = posProj * 0.5f + 0.5f; // convert to DNC

	if (posProj.z > 1.f) // fragment is outside of far plane of view frustum
		return 1.f;

	float bias = mix(1.f, 2.f, 1.f - clamp(dot(u_toLight, normalW), 0.f, 1.f)) * u_shadowBias;
	float depthL = texture(u_shadowMapArray, vec3(posProj.xy, cascadeIdx)).r;
	bool inShaow = posProj.z + bias > depthL;
	
	return inShaow ? 1.f - u_shadowStrength : 1.f;
}

subroutine (ShadowAttenType)
float softShadow(vec3 posW, vec3 normalW) {
	int cascadeIdx = calcCascadeIndex(z_Proj);
	vec4 posL = u_lightVP[cascadeIdx] * vec4(posW, 1.f); // convert to light space
	vec3 posProj = posL.xyz / posL.w;
	posProj = posProj * 0.5f + 0.5f; // convert to DNC

	if (posProj.z > 1.f) // fragment is outside of far plane of view frustum
		return 1.f;

	float bias = mix(1.f, 2.f, 1.f - clamp(dot(u_toLight, normalW), 0.f, 1.f)) * u_shadowBias;
	vec2 texelSize = vec2(1.f / textureSize(u_shadowMapArray, 0));
	int halfCornelSize = int(PCFCornelSize / 2);
	int shadowArea = 0;
	for (int x = -halfCornelSize; x <= halfCornelSize; x++) {
		for (int y = -halfCornelSize; y <= halfCornelSize; y++) {
			float depthL = texture(u_shadowMapArray, vec3(posProj.xy + vec2(x, y) * texelSize, cascadeIdx)).r;
			shadowArea += posProj.z + bias > depthL ? 1 : 0;
		}
	}

	float atten = mix(1.f - u_shadowStrength, 1.f, 1.f - shadowArea / float(PCFCornelSize * PCFCornelSize));
	return atten;
}


vec4 calcDirectionalLight(in vec3 diffuseTexColor, in vec3 specluarTexColor, in vec3 emissiveTexColor) {
	// diffuse
	float dotL = clamp(dot(normalize(u_toLight), normalize(normal_W)), 0.f, 1.f);
	vec3 diffuse = u_lightColor.rgb * u_diffuseFactor.rgb * diffuseTexColor * u_lightColor.a * dotL;

	// specular
	float dotV = clamp(dot(normalize(normalize(u_cameraPosW - pos_W) + normalize(u_toLight)), normalize(normal_W)), 0.f, 1.f);
	vec3 specular = u_lightColor.rgb * u_specularFactor.rgb * specluarTexColor * u_lightColor.a * pow(dotV, u_specularFactor.a);

	// emissive
	vec3 emissive = u_emissiveColor * emissiveTexColor;

	float shaodwAttn = u_shadowAtten(pos_W, normal_W);

	return vec4((diffuse + specular) * shaodwAttn + emissive, 1.f);
}

out vec4 frag_color;

void main() {
	vec3 diffuseTexColor = u_hasDiffuseMap ? sRGB2RGB(texture(u_diffuseMap, f_uv).rgb) : vec3(1.f);
	vec3 specularTexColor = u_hasSpecularMap ? sRGB2RGB(texture(u_specularMap, f_uv).rgb) : vec3(1.f);
	vec3 emissiveTexColor = u_hasEmissiveMap ? sRGB2RGB(texture(u_emissiveMap, f_uv).rgb) : vec3(0.f);
	
	frag_color = calcDirectionalLight(diffuseTexColor, specularTexColor, emissiveTexColor);
}


