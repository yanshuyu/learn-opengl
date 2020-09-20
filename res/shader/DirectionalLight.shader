#shader vertex
#version 430 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 4) in vec2 a_uv;


out vec3 pos_W;
out vec3 normal_W;
out vec2 f_uv;


layout(location = 0) uniform mat4 u_MVP;
layout(location = 1) uniform mat4 u_ModelMat;

invariant gl_Position;

void main() {
	gl_Position = u_MVP * vec4(a_pos, 1.f);
	pos_W = (u_ModelMat * vec4(a_pos, 1.f)).xyz;
	normal_W = (u_ModelMat * vec4(a_normal, 0.f)).xyz;
	f_uv = a_uv;
}


/////////////////////////////////////////////////////////////////////


#shader fragment
#version 430 core

in vec3 pos_W;
in vec3 normal_W;
in vec2 f_uv;


layout(location = 2) uniform sampler2D u_diffuseMap;
layout(location = 3) uniform bool u_hasDiffuseMap;

layout(location = 4) uniform sampler2D u_emissiveMap;
layout(location = 5) uniform bool u_hasEmissiveMap;

layout(location = 6) uniform vec3 u_cameraPosW;

layout(std140) uniform LightBlock {
	vec4 u_lightColor; //(a for intensity)
	vec3 u_toLight;
};


layout(std140) uniform MatrialBlock {
	vec4 u_diffuseFactor; //(a for alpha)
	vec4 u_specularFactor; // (a for shininess)
	vec3 u_emissiveColor; // (a for embient absord)
};

out vec4 frag_color;


vec4 calcDirectionalLight(in vec3 diffuseTexColor, in vec3 emissiveTexColor) {
	// diffuse
	float dotL = clamp(dot(normalize(u_toLight), normalize(normal_W)), 0.f, 1.f);
	vec3 diffuse = u_lightColor.rgb * u_diffuseFactor.rgb * diffuseTexColor * u_lightColor.a * dotL;

	// specular
	float dotV = clamp(dot(normalize(normalize(u_cameraPosW - pos_W) + normalize(u_toLight)), normalize(normal_W)), 0.f, 1.f);
	vec3 specular = u_lightColor.rgb * u_specularFactor.rgb * u_lightColor.a * pow(dotV, u_specularFactor.a);

	// emissive
	vec3 emissive = u_emissiveColor * emissiveTexColor;

	return vec4(diffuse + specular + emissive, 1.f);
}


void main() {
	vec3 diffuseTexColor = u_hasDiffuseMap ? texture(u_diffuseMap, f_uv).rgb : vec3(1.f);
	vec3 emissiveTexColor = u_hasEmissiveMap ? texture(u_emissiveMap, f_uv).rgb : vec3(0.f);
	
	frag_color = calcDirectionalLight(diffuseTexColor, emissiveTexColor);
}


