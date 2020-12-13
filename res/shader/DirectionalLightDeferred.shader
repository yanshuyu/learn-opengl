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

#define MAXNUMCASCADE 4
#define PCFCornelSize 3

in vec2 f_uv;


layout(location = 0) uniform sampler2D u_posW;
layout(location = 1) uniform sampler2D u_nromalW;
layout(location = 2) uniform sampler2D u_diffuse;
layout(location = 3) uniform sampler2D u_specular;
layout(location = 4) uniform sampler2D u_emissive;

layout(location = 5) uniform float u_maxShininess;
layout(location = 6) uniform vec3 u_cameraPosW;

layout(location = 8) uniform sampler2DArray u_shadowMapArray;

layout(location = 9) uniform mat4 u_lightVP[MAXNUMCASCADE];
layout(location = 13) uniform float u_cascadesFarZ[MAXNUMCASCADE];
layout(location = 17) uniform int u_numCascade;

layout(location = 18) uniform mat4 u_VPMat; // to projection space
layout(location = 19) uniform float u_shadowStrength;
layout(location = 20) uniform float u_shadowBias;


layout(std140) uniform LightBlock{
	vec4 u_lightColor; //(a for intensity)
	vec3 u_toLight;
};


subroutine float ShadowAttenType(vec3 posW, vec3 normalW);
subroutine uniform ShadowAttenType u_shadowAtten;


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

subroutine(ShadowAttenType)
float noShadow(vec3 posW, vec3 normalW) {
	return 1.f;
}

subroutine(ShadowAttenType)
float hardShadow(vec3 posW, vec3 normalW) {
	vec3 posProj = vec3(u_VPMat * vec4(posW, 1.f));
	int cascadeIdx = calcCascadeIndex(posProj.z);
	vec4 posL = u_lightVP[cascadeIdx] * vec4(posW, 1.f); // convert to light space
	vec3 posNDC = posL.xyz / posL.w;
	posNDC = posNDC * 0.5f + 0.5f; // convert to DNC

	if (posNDC.z > 1.f) // fragment is outside of far plane of view frustum
		return 1.f;

	float bias = mix(1.f, 2.f, 1.f - clamp(dot(u_toLight, normalW), 0.f, 1.f)) * u_shadowBias;
	float depthL = texture(u_shadowMapArray, vec3(posNDC.xy, cascadeIdx)).r;
	bool inShaow = posNDC.z + bias > depthL;
	return inShaow ? 1.f - u_shadowStrength : 1.f;
}

subroutine(ShadowAttenType)
float softShadow(vec3 posW, vec3 normalW) {
	vec3 posProj = vec3(u_VPMat * vec4(posW, 1.f));
	int cascadeIdx = calcCascadeIndex(posProj.z);
	vec4 posL = u_lightVP[cascadeIdx] * vec4(posW, 1.f); // convert to light space
	vec3 posNDC = posL.xyz / posL.w;
	posNDC = posNDC * 0.5f + 0.5f; // convert to DNC

	if (posNDC.z > 1.f) // fragment is outside of far plane of view frustum
		return 1.f;

	float bias = mix(1.f, 2.f, 1.f - clamp(dot(u_toLight, normalW), 0.f, 1.f)) * u_shadowBias;
	vec2 texelSize = vec2(1.f / textureSize(u_shadowMapArray, 0));
	int halfCornelSize = int(PCFCornelSize / 2);
	int shadowArea = 0;
	for (int x = -halfCornelSize; x <= halfCornelSize; x++) {
		for (int y = -halfCornelSize; y <= halfCornelSize; y++) {
			float depthL = texture(u_shadowMapArray, vec3(posNDC.xy + vec2(x, y) * texelSize, cascadeIdx)).r;
			shadowArea += posNDC.z + bias > depthL ? 1 : 0;
		}
	}

	float atten = mix(1.f - u_shadowStrength, 1.f, 1.f - shadowArea / float(PCFCornelSize * PCFCornelSize));
	return atten;
}


vec4 calcDirectionalLight(in vec3 diffuse, 
						in vec3 specular, 
						in vec3 emissive, 
						in float shininess,
						in vec3 posW, 
						in vec3 normalW) {
	// diffuse
	float dotL = clamp(dot(normalize(u_toLight), normalW), 0.f, 1.f);
	vec3 d = u_lightColor.rgb * diffuse * u_lightColor.a * dotL;

	// specular
	float dotV = clamp(dot(normalize(normalize(u_cameraPosW - posW) + normalize(u_toLight)), normalW), 0.f, 1.f);
	vec3 s = u_lightColor.rgb * specular  * u_lightColor.a * pow(dotV, shininess);

	float shaodwAttn = u_shadowAtten(posW, normalW);

	return vec4((d + s) * shaodwAttn + emissive, 1.f);
}


out vec4 frag_color;

void main() {
	vec3 posW = texture(u_posW, f_uv).xyz;
	vec3 normalW = (texture(u_nromalW, f_uv).xyz - 0.5) * 2;
	vec3 diffuse = texture(u_diffuse, f_uv).rgb;
	vec3 specular = texture(u_specular, f_uv).rgb;
	vec3 emissive = texture(u_emissive, f_uv).rgb;
	float shininess = texture(u_specular, f_uv).a * u_maxShininess;

	frag_color = calcDirectionalLight(diffuse, specular, emissive, shininess, posW, normalW);
}


