#shader compute
#version 450 core


layout(local_size_x = 256) in;

layout(std430) readonly buffer Hist {
	uint b_Hist[];
};

layout(r16f) uniform image2D u_OutAveLum;

uniform uint u_NumPixel;

shared uint lumSumShared[256];


void main() {
	lumSumShared[gl_LocalInvocationIndex] = b_Hist[gl_LocalInvocationIndex] * gl_LocalInvocationIndex;
	groupMemoryBarrier();

	for (uint i = 256 >> 1; i > 0; i >>= 1) {
		if (gl_LocalInvocationIndex < i) {
			lumSumShared[gl_LocalInvocationIndex] += lumSumShared[gl_LocalInvocationIndex + i];
		}
		groupMemoryBarrier();
	}

	if (gl_LocalInvocationIndex == 0) {
		float aveLum = (lumSumShared[0] / float(u_NumPixel) - 1.0) / 254.0;
		imageStore(u_OutAveLum, ivec2(0, 0), vec4(exp(aveLum), 0.f, 0.f, 0.f));
	}
};



