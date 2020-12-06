#shader vertex
#version 450 core


layout(location = 0) in vec3 a_pos;

out vec3 frag_uv;

uniform mat4 u_VP;

void main() {
	frag_uv = a_pos;
	vec4 pos = u_VP * vec4(a_pos, 1.f); // ensure remove translation form mvp
	gl_Position = pos.xyww; //set all sky box vertex's depth to 1.0
}



///////////////////////////////////////


#shader fragment
#version 450 core

in vec3 frag_uv;
out vec4 frag_color;

uniform samplerCube u_CubeMap;

void main() {
	frag_color = vec4(texture(u_CubeMap, frag_uv).rgb, 1.f);
}