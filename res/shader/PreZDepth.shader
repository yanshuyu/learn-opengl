#shader vertex
#version 450 core

layout(location = 0) in vec3 a_pos;

layout(location = 0) uniform mat4 u_MVP;

invariant gl_Position;

void main() {
	gl_Position = u_MVP * vec4(a_pos, 1.0);
}


///////////////////// (output depth for debuging)


#shader fragment
#version 450 core

layout(location = 1) uniform float u_near;
layout(location = 2) uniform float u_far;

layout(location = 0) out vec4 frag_color;



void LinearizeDepth(in float screenDepth, out float viewDepth)
{
	float z = screenDepth * 2.0 - 1.0; // back to NDC 
	viewDepth = (2.0 * u_near * u_far) / (u_far + u_near - z * (u_far - u_near));
}


void main() {
	float viewDepth = u_far;
	LinearizeDepth(gl_FragCoord.z, viewDepth);
	frag_color = vec4(vec3(viewDepth / u_far), 1.0);
}
