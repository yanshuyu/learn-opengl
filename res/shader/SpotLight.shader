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


uniform mat4 u_VPMat;
uniform mat4 u_ModelMat;
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



//////////////////////////////////////////////////////////



#shader fragment
#version 450 core

#define HARD_SHADOW 1
#define SOFT_SHADOW	2

const int PCFCornelSize = 5;

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


vec4 calcSpotLight(in vec3 normal, in vec3 diffuseTexColor, in vec3 specularTexColor, in vec3 emissiveTexColor) {
	// diffuse
	vec3 toLight = normalize(u_toLight);
	float rangeAtten = 1.f - smoothstep(0.f, u_lightPos.w, distance(u_lightPos.xyz, fs_in.pos_W));
	float angleAtten = smoothstep(u_angles.x, u_angles.y, acos(clamp(dot(normalize(fs_in.pos_W - u_lightPos.xyz), -toLight), 0.f, 1.f)));

	float dotL = clamp(dot(toLight, normal), 0.f, 1.f);
	vec3 d = u_lightColor.rgb * u_diffuseFactor.rgb * diffuseTexColor * u_lightColor.a * dotL * rangeAtten * angleAtten;

	//specular
	vec3 toView = normalize(u_cameraPosW - fs_in.pos_W);
	float dotV = clamp(dot(normalize(toLight + toView), normal), 0.f, 1.f);
	vec3 s = u_lightColor.rgb * u_specularFactor.rgb * specularTexColor * u_lightColor.a * pow(dotV, u_specularFactor.a) * rangeAtten * angleAtten;

	// emissive
	vec3 e = emissiveTexColor * u_emissiveColor;

	// shadow
	float shadowAtten = calcShadowAtten(fs_in.pos_W, normal);

	return vec4((d + s) * shadowAtten + e, 1.f);
}


out vec4 frag_color;


void main() {
	vec4 diffuseTexColor = u_hasDiffuseMap ? texture(u_diffuseMap, fs_in.uv) : vec4(1.f);
	if (diffuseTexColor.a < 0.05f)
		discard;
	
	diffuseTexColor.rgb = sRGB2RGB(diffuseTexColor.rgb);

	vec3 specTexColor = u_hasSpecularMap ? sRGB2RGB(texture(u_specularMap, fs_in.uv).rgb) : vec3(1.f);
	vec3 emissiveTexColor = u_hasEmissiveMap ? sRGB2RGB(texture(u_emissiveMap, fs_in.uv).rgb) : vec3(1.f);
	vec3 normalW = fs_in.normal_W;

	if (u_hasNormalMap) {
		vec3 normal = (texture(u_normalMap, fs_in.uv).xyz - 0.5) * 2;
		vec3 biTangent = normalize(cross(fs_in.normal_W, fs_in.tangent_W));
		normalW = normalize(mat3(fs_in.tangent_W, biTangent, fs_in.normal_W) * normal);
	}
	
	frag_color = calcSpotLight(normalW,  diffuseTexColor.rgb, specTexColor, emissiveTexColor);
}