#shader vertex
#version 450 core

in vec3 v_pos;

uniform mat4 u_MVP;

void main() {
	gl_Position = u_MVP * vec4(v_pos, 1.0);
}



#shader fragment
#version 450 core

uniform vec4 u_Color;

out vec4 o_Color;

void main() {
	o_Color = u_Color;
}