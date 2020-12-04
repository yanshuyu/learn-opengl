#shader vertex
#version 450 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;

out vec2 frag_uv;

void main() {
	gl_Position = vec4(a_pos, 1.f);
	frag_uv = a_uv;
}


//////////////////////////////////////


#shader fragment
#version 450 core

in vec2 frag_uv;
out vec4 frag_color;

uniform sampler2D u_hdrTexture;
uniform float u_lumAve;
uniform float u_exposure;
uniform float u_white;


uniform mat3 rgb2xyz = mat3(
	0.4124564, 0.2126729, 0.0193339,
	0.3575761, 0.7151522, 0.1191920,
	0.1804375, 0.0721750, 0.9503041);

uniform mat3 xyz2rgb = mat3(
	3.2404542, -0.9692660, 0.0556434,
	-1.5371385, 1.8760108, -0.2040259,
	-0.4985314, 0.0415560, 1.0572252);


// convert from RGB to CIE-XYZ, then to CIE-xyY
// modify luminance
// convert from CIE-xyY to CIE-XYZ, then to RGB
vec3 ToneMapping(vec3 hdrColor) {
	vec3 xyzColor = rgb2xyz * hdrColor;
	
	float xyzSum = xyzColor.x + xyzColor.y + xyzColor.z;
	
	vec3 xyYColor = vec3(xyzColor.x / xyzSum, xyzColor.y / xyzSum, xyzColor.y);
	
	float L = (u_exposure * xyYColor.z) / u_lumAve;
	L = (L * (1 + L / (u_white * u_white))) / (1 + L); // new luminance

	xyzColor.x = (L * xyYColor.x) / (xyYColor.y);
	xyzColor.y = L;
	xyzColor.z = (L * (1 - xyYColor.x - xyYColor.y)) / xyYColor.y;

	return xyz2rgb * xyzColor;
}

void main() {
	vec4 hdrColor = texture(u_hdrTexture, frag_uv);
	frag_color = vec4(ToneMapping(hdrColor.rgb), 1.f);
}