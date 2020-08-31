#shader vertex
#version 450 core

layout(location = 0) in vec2 v_pos;

void main() {
	gl_Position = vec4(v_pos, 0.0, 1.0);
}



#shader fragment
#version 450 core

layout(location = 0) uniform vec4 u_mainColor;

layout(location = 0) out vec4 p_color;

void main() {
	p_color = u_mainColor;
}