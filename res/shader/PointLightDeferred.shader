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

const float diskRadius = 0.05f;

const vec3 sampleOffsetDirections[20] = {
	vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
	vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
	vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
	vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
	vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
};



in vec2 f_uv;



layout(location = 0) uniform sampler2D u_posW;
layout(location = 1) uniform sampler2D u_nromalW;
layout(location = 2) uniform sampler2D u_diffuse;
layout(location = 3) uniform sampler2D u_specular;
layout(location = 4) uniform sampler2D u_emissive;

layout(location = 5) uniform float u_maxShininess;
layout(location = 6) uniform vec3 u_cameraPosW;

layout(location = 7) uniform samplerCube u_shadowMap;
layout(location = 8) uniform float u_shadowStrength;
layout(location = 9) uniform float u_shadowBias;

subroutine float ShadowAttenType(vec3 posW);
subroutine uniform ShadowAttenType u_shadowAtten;


layout(std140) uniform LightBlock{
	vec4 u_lightPos; // (w for range)
	vec4 u_lightColor; //(a for intensity)
};


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
	return depth - u_shadowBias > closestDepth ? (1.f - u_shadowStrength) : 1.f;
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
		shadow += depth - u_shadowBias > closestDepth ? 1.f : 0.f;
	}

	return mix(1.f - u_shadowStrength, 1.f, 1.f - shadow / 20.f);
}

vec4 calcPointLight(in vec3 diffuse,
					in vec3 specular,
					in vec3 emissive,
					in float shininess,
					in vec3 posW,
					in vec3 normalW) {
	// diffuse
	vec3 toLight = u_lightPos.xyz - posW;
	float rangeAtten = 1.f - smoothstep(0.f, u_lightPos.w, length(toLight));
	toLight = normalize(toLight);
	float dotL = clamp(dot(toLight, normalW), 0.f, 1.f);
	vec3 d = u_lightColor.rgb * diffuse  * u_lightColor.a * dotL * rangeAtten;

	//specular
	vec3 toView = normalize(u_cameraPosW - posW);
	float dotV = clamp(dot(normalize(toLight + toView), normalW), 0.f, 1.f);
	vec3 s = u_lightColor.rgb * specular * u_lightColor.a * pow(dotV, shininess) * rangeAtten;

	float shadowAtten = u_shadowAtten(posW);

	return vec4((d + s) * shadowAtten + emissive, 1.f);
}


out vec4 frag_color;

void main() {
	vec3 posW = texture(u_posW, f_uv).xyz;
	vec3 normalW = (texture(u_nromalW, f_uv).xyz - 0.5) * 2;
	vec3 diffuse = texture(u_diffuse, f_uv).rgb;
	vec3 specular = texture(u_specular, f_uv).rgb;
	vec3 emissive = texture(u_emissive, f_uv).rgb;
	float shininess = texture(u_specular, f_uv).a * u_maxShininess;

	frag_color = calcPointLight(diffuse, specular, emissive, shininess, posW, normalW);
}


