#shader vertex
#version 430 core

layout(location = 0) in vec3 a_pos;


void main() {
	gl_Position = vec4(a_pos, 1.f);
}



/////////////////////////////////////////



#shader fragment
#version 430 core

#define MAX_FRAG_PER_PIXEL 32

out vec4 frag_Color;

struct Fragment { // an entry in piexl frag link list
	vec4 color;
	float depth;
	int next;
};

layout(std430) readonly buffer FragmentBuffer { // ssao to store all frags
	Fragment b_Frags[];
};

layout(r32ui) uniform readonly uimage2D u_FragHeader;

layout(rgba16f) uniform readonly image2D u_BackroundImage; // back ground image composite all fragments into


Fragment fragList[MAX_FRAG_PER_PIXEL]; // fragment list fectch from buffer


int FetchPixelFragments() {
	int numFrag = 0;
	uint idx = imageLoad(u_FragHeader, ivec2(gl_FragCoord.xy)).r;
	
	while (idx != 0xffffffff && numFrag < MAX_FRAG_PER_PIXEL) {
		fragList[numFrag] = b_Frags[idx];
		idx = b_Frags[idx].next;
		numFrag++;
	}

	
	// insertion sort frags in back to front order by depth 
	int i = 1;
	while (i < numFrag) {
		Fragment inserted = fragList[i];
		
		int j = i;
		while (j > 0 && fragList[j-1].depth < inserted.depth) {
			fragList[j] = fragList[j - 1];
			j--;
		}

		fragList[j] = inserted;

		i++;
	}

	
	return numFrag;
}

vec3 BlendFragments(int numFrag) {
	vec3 C = imageLoad(u_BackroundImage, ivec2(gl_FragCoord.xy)).rgb;
	
	for (int i = 0; i < numFrag; i++) {
		C = mix(C, fragList[i].color.rgb, fragList[i].color.a);
	}

	return C;
}

void main() {
	int numFrag = FetchPixelFragments();
	vec3 C = BlendFragments(numFrag);

	frag_Color = vec4(C, 1.f);
}