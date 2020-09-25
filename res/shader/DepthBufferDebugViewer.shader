
#shader vertex
#version 450 core

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;

layout(location = 0) uniform mat4 u_MVP;

out vec2 f_uv;

void main() {
	gl_Position = u_MVP * vec4(a_pos, 0.f, 1.f);
	f_uv = a_uv;
}


/////////////////////////////////////////


#shader fragment
#version 450 core

in vec2 f_uv;

layout(location = 1) uniform sampler2D u_depthTexture;
layout(location = 2) uniform float u_near;
layout(location = 3) uniform float u_far;

out vec4 frag_color;

float LinearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0; // back to NDC 
	return (2.0 * u_near * u_far) / (u_far + u_near - z * (u_far - u_near));
}



void main() {
	float depth = LinearizeDepth(texture(u_depthTexture, f_uv).r) / u_far;
	frag_color = vec4(vec3(depth), 1.f);
}