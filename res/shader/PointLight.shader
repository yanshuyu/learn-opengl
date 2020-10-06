#shader vertex
#version 450 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 4) in vec2 a_uv;

layout(location = 0) uniform mat4 u_VPMat;
layout(location = 1) uniform mat4 u_ModelMat;

out vec3 pos_W;
out vec3 normal_W;
out vec2 f_uv;

invariant gl_Position;


void main() {
	gl_Position = u_VPMat * u_ModelMat * vec4(a_pos, 1.f);
	pos_W = (u_ModelMat * vec4(a_pos, 1.f)).xyz;
	normal_W = (u_ModelMat * vec4(a_normal, 0.f)).xyz;
	f_uv = a_uv;
}



///////////////////////////////////////////////////////



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




in vec3 pos_W;
in vec3 normal_W;
in vec2 f_uv;


layout(location = 2) uniform sampler2D u_diffuseMap;
layout(location = 3) uniform bool u_hasDiffuseMap;

layout(location = 4) uniform sampler2D u_normalMap;
layout(location = 5) uniform bool u_hasNormalMap;

layout(location = 6) uniform sampler2D u_specularMap;
layout(location = 7) uniform bool u_hasSpecularMap;

layout(location = 8) uniform sampler2D u_emissiveMap;
layout(location = 9) uniform bool u_hasEmissiveMap;

layout(location = 10) uniform vec3 u_cameraPosW;

layout(location = 11) uniform samplerCube u_shadowMap;
layout(location = 12) uniform float u_shadowStrength;
layout(location = 13) uniform float u_shadowBias;

subroutine float ShadowAttenType(vec3 posW);
subroutine uniform ShadowAttenType u_shadowAtten;

layout(std140) uniform LightBlock{
	vec4 u_lightPos; // (w for range)
	vec4 u_lightColor; //(a for intensity)
};


layout(std140) uniform MatrialBlock{
	vec4 u_diffuseFactor; //(a for alpha)
	vec4 u_specularFactor; // (a for shininess)
	vec3 u_emissiveColor;
};


vec3 sRGB2RGB(in vec3 color) {
	return color * color;
}

subroutine (ShadowAttenType)
float noShadow(vec3 posW) {
	return 1.f;
}

subroutine (ShadowAttenType)
float hardShadow(vec3 posW) {
	vec3 l2v = posW - u_lightPos.xyz;
	float depth = length(l2v) / u_lightPos.w;
	if (depth > 1.f)
		return 1.f;

	float closestDepth = texture(u_shadowMap, l2v).r;
	return depth - u_shadowBias > closestDepth ? (1.f - u_shadowStrength) : 1.f;
}

subroutine (ShadowAttenType)
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


vec4 calcPointLight(in vec3 diffuseTexColor, in vec3 specularTexColor , in vec3 emissiveTexColor) {
	// diffuse
	vec3 toLight = u_lightPos.xyz - pos_W;
	float rangeAtten = 1.f - smoothstep(0.f, u_lightPos.w, length(toLight));
	toLight = normalize(toLight);
	float dotL = clamp( dot(toLight, normalize(normal_W)), 0.f, 1.f );
	vec3 diffuse = u_lightColor.rgb * u_diffuseFactor.rgb * diffuseTexColor * u_lightColor.a * dotL * rangeAtten;

	//specular
	vec3 toView = normalize(u_cameraPosW - pos_W);
	float dotV = clamp( dot(normalize( toLight + toView), normalize(normal_W)), 0.f, 1.f);
	vec3 specular = u_lightColor.rgb * u_specularFactor.rgb * specularTexColor * u_lightColor.a * pow(dotV, u_specularFactor.a) * rangeAtten;

	// emissive
	vec3 emissive = emissiveTexColor * u_emissiveColor;

	float shadowAtten = u_shadowAtten(pos_W);

	return vec4((diffuse + specular) * shadowAtten + emissive, 1.f);
}


out vec4 frag_color;


void main() {
	vec3 diffuseTexColor = u_hasDiffuseMap ? sRGB2RGB(texture(u_diffuseMap, f_uv).rgb) : vec3(1.f);
	vec3 sepcTexColor = u_hasSpecularMap ? sRGB2RGB(texture(u_specularMap, f_uv).rgb) : vec3(1.f);
	vec3 emissiveTexColor = u_hasEmissiveMap ? sRGB2RGB(texture(u_emissiveMap, f_uv).rgb) : vec3(0.f);
	frag_color = calcPointLight(diffuseTexColor, sepcTexColor, emissiveTexColor);
}