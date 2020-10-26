#shader vertex
#version 450 core

#define MAX_NUM_BONE 156

layout(location = 0) in vec3 a_pos;
layout(location = 4) in ivec4 a_bones;
layout(location = 5) in vec4 a_weights;


uniform mat4 u_ModelMat;
uniform mat4 u_SkinPose[MAX_NUM_BONE];

subroutine vec4 TransformType(vec4 pos, ivec4 bones, vec4 weights);
subroutine uniform TransformType u_Transform;

subroutine(TransformType)
vec4 staticMesh(vec4 pos, ivec4 bones, vec4 weights) {
	return u_ModelMat * pos;
}

subroutine(TransformType)
vec4 skinMesh(vec4 pos, ivec4 bones, vec4 weights) {
	mat4 skinMat = u_SkinPose[bones.x] * weights.x
		+ u_SkinPose[bones.y] * weights.y
		+ u_SkinPose[bones.z] * weights.z
		+ u_SkinPose[bones.w] * weights.w;

	return u_ModelMat * skinMat * pos;
}


void main() {
	gl_Position = u_Transform(vec4(a_pos, 1.f), a_bones, a_weights);
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
