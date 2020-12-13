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

#define HARD_SHADOW 1
#define SOFT_SHADOW	2

const int PCFCornelSize = 5;

in vec2 f_uv;


layout(location = 0) uniform sampler2D u_posW;
layout(location = 1) uniform sampler2D u_nromalW;
layout(location = 2) uniform sampler2D u_diffuse;
layout(location = 3) uniform sampler2D u_specular;
layout(location = 4) uniform sampler2D u_emissive;

layout(location = 5) uniform float u_maxShininess;
layout(location = 6) uniform vec3 u_cameraPosW;

layout(location = 7) uniform sampler2D u_shadowMap;
layout(location = 8) uniform bool u_hasShadowMap;


layout(std140) uniform LightBlock{
	vec4 u_lightPos; // (wfor range)
	vec4 u_lightColor; //(a for intensity)
	vec3 u_toLight;
	vec2 u_angles;
};


layout(std140) uniform ShadowBlock{
	mat4 u_lightVP;
	float u_shadowStrength;
	float u_shadowBias;
	int u_shadowType;
};


out vec4 frag_color;


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
		
		} else if (u_shadowType == SOFT_SHADOW) {
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


vec4 calcSpotLight(in vec3 diffuse,
					in vec3 specular,
					in vec3 emissive,
					in float shininess,
					in vec3 posW,
					in vec3 normalW) {
	// diffuse
	vec3 toLight = normalize(u_toLight);
	float rangeAtten = 1.f - smoothstep(0.f, u_lightPos.w, distance(u_lightPos.xyz, posW));
	float angleAtten =  smoothstep(u_angles.x, u_angles.y, acos(clamp(dot(normalize(posW - u_lightPos.xyz), -toLight), 0.f, 1.f)));

	float dotL = clamp(dot(toLight, normalW), 0.f, 1.f);
	vec3 d = u_lightColor.rgb * diffuse * u_lightColor.a * dotL * rangeAtten * angleAtten;

	//specular
	vec3 toView = normalize(u_cameraPosW - posW);
	float dotV = clamp(dot(normalize(toLight + toView), normalW), 0.f, 1.f);
	vec3 s = u_lightColor.rgb * specular * u_lightColor.a * pow(dotV, shininess) * rangeAtten * angleAtten;

	// shadow
	float shadowAtten = calcShadowAtten(posW, normalW);

	return vec4((d + s) * shadowAtten + emissive, 1.f);
}



void main() {
	vec3 posW = texture(u_posW, f_uv).xyz;
	vec3 normalW = (texture(u_nromalW, f_uv).xyz - 0.5) * 2;
	vec3 diffuse = texture(u_diffuse, f_uv).rgb;
	vec3 specular = texture(u_specular, f_uv).rgb;
	vec3 emissive = texture(u_emissive, f_uv).rgb;
	float shininess = texture(u_specular, f_uv).a * u_maxShininess;

	frag_color = calcSpotLight(diffuse, specular, emissive, shininess, posW, normalW);
}


