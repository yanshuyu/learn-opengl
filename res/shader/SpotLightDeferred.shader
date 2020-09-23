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

in vec2 f_uv;


layout(location = 0) uniform sampler2D u_posW;
layout(location = 1) uniform sampler2D u_nromalW;
layout(location = 2) uniform sampler2D u_diffuse;
layout(location = 3) uniform sampler2D u_specular;
layout(location = 4) uniform sampler2D u_emissive;

layout(location = 5) uniform float u_maxShininess;
layout(location = 6) uniform vec3 u_cameraPosW;


layout(std140) uniform LightBlock{
	vec4 u_lightPos; // (wfor range)
	vec4 u_lightColor; //(a for intensity)
	vec3 u_toLight;
	vec2 u_angles;
};


out vec4 frag_color;


vec4 calcSpotLight(in vec3 diffuse,
					in vec3 specular,
					in vec3 emissive,
					in float shininess,
					in vec3 posW,
					in vec3 normalW) {
	// diffuse
	vec3 toLight = normalize(u_toLight);
	float rangeAtten = 1.f - smoothstep(0.f, u_lightPos.w, distance(u_lightPos.xyz, posW));
	float angleAtten = smoothstep(u_angles.x, u_angles.y, acos(dot(normalize(posW - u_lightPos.xyz), -toLight)));
	float dotL = clamp(dot(toLight, normalW), 0.f, 1.f);
	vec3 d = u_lightColor.rgb * diffuse * u_lightColor.a * dotL * rangeAtten * angleAtten;

	//specular
	vec3 toView = normalize(u_cameraPosW - posW);
	float dotV = clamp(dot(normalize(toLight + toView), normalW), 0.f, 1.f);
	vec3 s = u_lightColor.rgb * specular * u_lightColor.a * pow(dotV, shininess) * rangeAtten * angleAtten;

	return vec4(d + s + emissive, 1.f);
}

void main() {
	vec3 posW = texture(u_posW, f_uv).xyz;
	vec3 normalW = texture(u_nromalW, f_uv).xyz;
	vec3 diffuse = texture(u_diffuse, f_uv).rgb;
	vec3 specular = texture(u_specular, f_uv).rgb;
	vec3 emissive = texture(u_emissive, f_uv).rgb;
	float shininess = texture(u_specular, f_uv).a * u_maxShininess;

	frag_color = calcSpotLight(diffuse, specular, emissive, shininess, posW, normalW);
}


