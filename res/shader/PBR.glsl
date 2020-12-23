/*
 PBR reflection equation: L(p, wo) = กา BRDF(l, v) * L(p, wi) * NdotL * dwi
 wo: radiance along wo(view) direction
 wi: irradiance along wi(light) direction
 BRDF: ration of irradiance to radiance

 BRDF = kd * F(lambert) + ks * F(cooktorrance)
      = kd * F(lambert) + ks * NFG / 4(wodotn, widotn)
*/

const float PI = 3.14159265359;


/*
 N for normal distribution function, statistically approximates the relative surface area of microfacets 
 exactly aligned to the halfway vector
*/

float NormalDistributionGGX(vec3 N, vec3 H, float roughness) {
    float roughness2 = roughness * roughness;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (roughness2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return roughness2 / max(denom, 0.001); // prevent divide by zero for roughness=0.0 and NdotH=1.0
}



/*
 G for geometry function, itstatistically approximates the relative surface area
 where its micro surface-details overshadow each other, causing light rays to be occluded.  
*/

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}


/*
 F for fresnel equation, it describes the ratio of light that gets reflected over the light that gets refracted
*/

vec3 FresnelSchlick(vec3 V, vec3 H, vec3 F0) {
    float cosTheta = clamp(dot(H, V), 0.f, 1.f);
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}



vec3 PBR(vec3 irradiance, vec3 L, vec3 N, vec3 V, vec3 albedo, float metallic, float roughness) {
    vec3 H = normalize(L + V);
    float NdotL = max(dot(N, L), 0.f);
    float NdotV = max(dot(N, V), 0.f);

    // calculate reflectance at normal incidence, if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
    
    float D = NormalDistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(V, H, F0);

    vec3 specular = D * G * F / max(4 * NdotL * NdotV, 0.001f);
    
    vec3 ks = F;
    vec3 kd = vec3(1.f) - ks; 

    // multiply kd by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).

    kd *= 1.0 - metallic;	
    vec3 diffuse = kd * albedo / PI;

    vec3 radiance = (diffuse + specular) * irradiance * NdotL;

    // HDR tone mapping
    //radiance = radiance / (radiance + vec3(1.f));
    
    // Gamma correction
    radiance = pow(radiance, vec3(1.f / 2.2f));

    return radiance;
}