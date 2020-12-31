

#define DIRECTIONAL 1
#define POINT 2
#define SPOT 3
#define AMBIENT 4 

#include "Phong.glsl"
#include "PBR.glsl"

struct Light {
	vec4 position; // (xyz)position (w)range
	vec4 color; // (rgb)color (a)intensity
	vec3 direction;
	vec3 angles; // spot angles/ ambient ground color
	int type;
};


layout(std430) buffer Lights {
	Light b_Lights[];
};

uniform int u_NumLights;


vec3 PhongLightting(vec3 Pos, vec3 N, vec3 Eye, vec3 A, vec3 S, float shinness) {
	vec3 I, L, V, C;

	V = normalize(Eye - Pos);

	for (int i = 0; i < u_NumLights; i++) {
		switch (b_Lights[i].type) {
		case DIRECTIONAL: {
			L = b_Lights[i].direction;
			I = b_Lights[i].color.rgb * b_Lights[i].color.a;
			C += Phong(I, L, N, V, A, S, shinness);
		} break;

		case POINT: {
			vec3 l = b_Lights[i].position.xyz - Pos;
			float distance = length(l);
			float rangeAtten = 1.f - smoothstep(0.f, b_Lights[i].position.w, distance);
			L = l / distance;
			I = b_Lights[i].color.rgb * b_Lights[i].color.a * rangeAtten;
			C += Phong(I, L, N, V, A, S, shinness);
		} break;

		case SPOT: {
			vec3 l = b_Lights[i].position.xyz - Pos;
			float distance = length(l);
			float rangeAtten = 1.f - smoothstep(0.f, b_Lights[i].position.w, distance);
			L = l / distance;
			float angleAtten = 1.f -  smoothstep(b_Lights[i].angles.x * 0.5f, b_Lights[i].angles.y * 0.5f, acos(dot(L, b_Lights[i].direction)));
			I = b_Lights[i].color.rgb * b_Lights[i].color.a * rangeAtten * angleAtten;
			C += Phong(I, L, N, V, A, S, shinness);
		} break;

		case AMBIENT: {
			vec3 HA = mix(b_Lights[i].angles, b_Lights[i].color.rgb, N.y * 0.5f + 0.5f) * b_Lights[i].color.a;
			C += HA * A;
		} break;

		}
	}
	
	return C;
}



vec3 PBRLightting(vec3 Pos, vec3 N, vec3 Eye, vec3 A, float M, float R) {
	vec3 I, L, V, C;

	V = normalize(Eye - Pos);
	
	for (int i = 0; i < u_NumLights; i++) {
		switch (b_Lights[i].type) {
		case DIRECTIONAL: {
			L = b_Lights[i].direction;
			I = b_Lights[i].color.rgb * b_Lights[i].color.a;
			C += PBR(I, L, N, V, A, M, R);
		} break;

		case POINT: {
			vec3 l = b_Lights[i].position.xyz - Pos;
			float distance = length(l);
			float rangeAtten = 1.f - smoothstep(0.f, b_Lights[i].position.w, distance);
			L = l / distance;
			I = b_Lights[i].color.rgb * b_Lights[i].color.a * rangeAtten;
			C += PBR(I, L, N, V, A, M, R);
		} break;

		case SPOT: {
			vec3 l = b_Lights[i].position.xyz - Pos;
			float distance = length(l);
			float rangeAtten = 1.f - smoothstep(0.f, b_Lights[i].position.w, distance);
			L = l / distance;
			float angleAtten = 1.f -  smoothstep(b_Lights[i].angles.x * 0.5f, b_Lights[i].angles.y * 0.5f, acos(dot(L, b_Lights[i].direction)));
			I = b_Lights[i].color.rgb * b_Lights[i].color.a * rangeAtten * angleAtten;
			C += PBR(I, L, N, V, A, M, R);
		} break;

		case AMBIENT: {
			vec3 HA = mix(b_Lights[i].angles, b_Lights[i].color.rgb, N.y * 0.5f + 0.5f) * b_Lights[i].color.a;
			C += HA * A;
		} break;

		}
	}
	
	return C;
}