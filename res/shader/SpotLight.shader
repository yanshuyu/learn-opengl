#shader vertex
#version 450 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 4) in vec2 a_uv;

layout(location = 0) uniform mat4 u_MVP;
layout(location = 1) uniform mat4 u_ModelMat;

out vec3 pos_W;
out vec3 normal_W;
out vec2 f_uv;

invariant gl_Position;

void main() {
	gl_Position = u_MVP * vec4(a_pos, 1.f);
	pos_W = (u_ModelMat * vec4(a_pos, 1.f)).xyz;
	normal_W = (u_ModelMat * vec4(a_normal, 0.f)).xyz;
	f_uv = a_uv;
}



//////////////////////////////////////////////////////////



#shader fragment
#version 450 core

#define HARD_SHADOW 1
#define SOFT_SHADOW	2

const int PCFCornelSize = 5;

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

layout(location = 10) uniform sampler2D u_shadowMap;
layout(location = 11) uniform bool u_hasShadowMap;


layout(location = 12) uniform vec3 u_cameraPosW;


layout(std140) uniform LightBlock{
	vec4 u_lightPos; // (wfor range)
	vec4 u_lightColor; //(a for intensity)
	vec3 u_toLight;
	vec2 u_angles;
};


layout(std140) uniform MatrialBlock{
	vec4 u_diffuseFactor; //(a for alpha)
	vec4 u_specularFactor; // (a for shininess)
	vec3 u_emissiveColor; // (a for embient absord)
};


layout(std140) uniform ShadowBlock{
	mat4 u_lightVP;
	float u_shadowStrength;
	float u_shadowBias;
	int u_shadowType;
};



vec3 sRGB2RGB(in vec3 color) {
	return color * color;
}


float calcShadowAtten(in vec3 posW, in vec3 normalW) {
	float shadowAtten = 1.f;
	vec4 posL = u_lightVP * vec4(posW, 1.f);
	vec3 posProj = posL.xyz / posL.w;
	posProj = posProj * 0.5f + 0.5f;
	float bias = mix(1.f, 2.f, 1.f - clamp(dot(u_toLight, normalW), 0.f, 1.f)) * u_shadowBias;

	if (u_hasShadowMap) {
		if (u_shadowType == HARD_SHADOW) {
			float depthL = texture(u_shadowMap, posProj.xy).r;
			bool inShaow = posProj.z + bias > depthL;
			shadowAtten = inShaow ? 1.f - u_shadowStrength : 1.f;

		}
		else if (u_shadowType == SOFT_SHADOW) {
			vec2 texelSize = 1.f / textureSize(u_shadowMap, 0);
			int halfCornelSize = PCFCornelSize / 2;
			int shadowArea = 0;

			for (int x = -halfCornelSize; x <= halfCornelSize; x++) {
				for (int y = -halfCornelSize; y <= halfCornelSize; y++) {
					float depthL = texture(u_shadowMap, posProj.xy + vec2(x, y) * texelSize).r;
					shadowArea += posProj.z + bias > depthL ? 1 : 0;
				}
			}

			shadowAtten = mix(1.f - u_shadowStrength, 1.f, 1.f - shadowArea / float(PCFCornelSize * PCFCornelSize));
		}
	}

	return shadowAtten;
}


vec4 calcSpotLight(in vec3 diffuseTexColor, in vec3 specularTexColor, in vec3 emissiveTexColor) {
	// diffuse
	vec3 toLight = normalize(u_toLight);
	float rangeAtten = 1.f - smoothstep(0.f, u_lightPos.w, distance(u_lightPos.xyz, pos_W));
	float angleAtten = smoothstep(u_angles.x, u_angles.y, acos(clamp(dot(normalize(pos_W - u_lightPos.xyz), -toLight), 0.f, 1.f)));

	float dotL = clamp(dot(toLight, normalize(normal_W)), 0.f, 1.f);
	vec3 d = u_lightColor.rgb * u_diffuseFactor.rgb * diffuseTexColor * u_lightColor.a * dotL * rangeAtten * angleAtten;

	//specular
	vec3 toView = normalize(u_cameraPosW - pos_W);
	float dotV = clamp(dot(normalize(toLight + toView), normalize(normal_W)), 0.f, 1.f);
	vec3 s = u_lightColor.rgb * u_specularFactor.rgb * specularTexColor * u_lightColor.a * pow(dotV, u_specularFactor.a) * rangeAtten * angleAtten;

	// emissive
	vec3 e = emissiveTexColor * u_emissiveColor;

	// shadow
	float shadowAtten = calcShadowAtten(pos_W, normal_W);

	return vec4((d + s) * shadowAtten + e, 1.f);
}


out vec4 frag_color;


void main() {
	vec3 diffuseTexColor = u_hasDiffuseMap ? sRGB2RGB(texture(u_diffuseMap, f_uv).rgb) : vec3(1.f);
	vec3 specTexColor = u_hasSpecularMap ? sRGB2RGB(texture(u_specularMap, f_uv).rgb) : vec3(1.f);
	vec3 emissiveTexColor = u_hasEmissiveMap ? sRGB2RGB(texture(u_emissiveMap, f_uv).rgb) : vec3(0.f);
	frag_color = calcSpotLight(diffuseTexColor, specTexColor, emissiveTexColor);
}