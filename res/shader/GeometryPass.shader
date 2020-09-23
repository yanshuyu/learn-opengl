#shader vertex
#version 450 core

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


///////////////////////////////////////////////////////////



#shader fragment
#version 450 core

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


layout(std140) uniform MatrialBlock{
	vec4 u_diffuseFactor; //(a for alpha)
	vec4 u_specularFactor; // (a for shininess)
	vec3 u_emissiveColor; 
};


layout(location = 0) out vec4 o_posW;
layout(location = 1) out vec4 o_normalW;
layout(location = 2) out vec4 o_diffuse;
layout(location = 3) out vec4 o_specular;
layout(location = 4) out vec4 o_emissive;


vec3 sRGB2RGB(in vec3 color) {
	return color * color;
}

void main() {
	vec3 dTex = u_hasDiffuseMap ? sRGB2RGB(texture(u_diffuseMap, f_uv).rgb) : vec3(1.f);
	vec3 sTex = u_hasSpecularMap ? sRGB2RGB(texture(u_specularMap, f_uv).rgb) : vec3(1.f);
	vec3 eTex = u_hasEmissiveMap ? sRGB2RGB(texture(u_emissiveMap, f_uv).rgb) : vec3(0.f);

	o_posW = vec4(pos_W, 1.f);
	o_normalW = vec4(normalize(normal_W), 0.f);
	o_diffuse = vec4(dTex * u_diffuseFactor.rgb, u_diffuseFactor.a);
	o_specular = vec4(sTex * u_specularFactor.rgb, u_specularFactor.a);
	o_emissive = vec4(eTex * u_emissiveColor, 0.f);
}
