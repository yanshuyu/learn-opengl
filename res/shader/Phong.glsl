

vec3 Phong(vec3 irradiance, vec3 L, vec3 N, vec3 V, vec3 kd, vec3 ks, float shinness) {
	vec3 H = normalize(L + V);
	float NdotL = max(dot(N, L), 0.f);
	float NdotH = max(dot(N, H), 0.f);
	
	// diffuse
	vec3 diffuse = irradiance * NdotL * kd;

	// specular
	vec3 specular = irradiance * ks * pow(NdotH, shinness);

	return diffuse + specular;
}