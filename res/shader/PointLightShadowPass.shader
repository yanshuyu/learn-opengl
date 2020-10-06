#shader vertex
#version 450 core

layout(location = 0) in vec3 a_pos;

uniform mat4 u_ModelMat;

void main() {
	gl_Position = u_ModelMat * vec4(a_pos, 1.f);
}


/////////////////////////////////////////////


#shader geometry
#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 u_lightVP[6];


void main() {
	for (int i = 0; i < 6; i++) { // each cube map face
		gl_Layer = i;
		for (int j = 0; j < 3; j++) { // each primitive vertex
			gl_Position = u_lightVP[i] * gl_in[j].gl_Position;
			EmitVertex();
		}
		EndPrimitive();
	}
}


//////////////////////////////////////////////


#shader fragment
#version 450 core
	
uniform float u_near;
uniform float u_far;

float LinearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0; // back to NDC 
	float d = (2.0 * u_near * u_far) / (u_far + u_near - z * (u_far - u_near));
	return d / u_far;
}


void main() {
	gl_FragDepth = LinearizeDepth(gl_FragCoord.z); // store liner depth to shadow map
}
