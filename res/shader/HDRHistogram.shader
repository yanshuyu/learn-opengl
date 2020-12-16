#shader compute
#version 450 core

#define EPSILON 0.05


layout(local_size_x = 16, local_size_y = 16) in;


layout(rgba16f) readonly uniform image2D u_InputImage; // input image for calculate histogram

layout(std430) buffer Hist { // buffer to store total histogram
	uint b_Hist[];
};

shared uint histShared[256]; // share memory to store local group histogram

layout(binding = 0, offset = 0) uniform atomic_uint u_pixelCounter;



uint RGB2LuminanceLevel(vec3 rgb) {
	float lum = log(dot(rgb, vec3(0.2125, 0.7154, 0.0721)));
	if (lum < EPSILON)
		return 0;

	lum = clamp(lum, 0.0, 1.0);
	
	return uint(lum * 254 + 1);
}



void main() {
	histShared[gl_LocalInvocationIndex] = 0;
	groupMemoryBarrier();

	ivec2 dim = imageSize(u_InputImage);
	ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
	if (all(lessThan(pix, dim))) {
		atomicCounterIncrement(u_pixelCounter);

		uint lum = RGB2LuminanceLevel(imageLoad(u_InputImage, pix).rgb);
		atomicAdd(histShared[lum], 1);
		
		groupMemoryBarrier(); // sync all invocation in this group

		atomicAdd(b_Hist[gl_LocalInvocationIndex], histShared[gl_LocalInvocationIndex]); // add local hist to global hist
	}
}



