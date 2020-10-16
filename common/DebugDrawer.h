#pragma once
#include"Interpolation.h"
#include"KeyFrameTrack.h"
#include<glm/glm.hpp>
#include<memory>


class Texture;
class Buffer;
class VertexArray;
class Pose;

class DebugDrawer {
public:
	static void setup();
	static void clenup();
	static void drawTexture(Texture* texture, const glm::vec2& windowSz, const glm::vec4& rect);
	static void drawScalarTrack(ScalarTrack* track, float numCycle, float numSamplePerCycle, LoopType loop, float xoffset, float yoffset, float sampleWidth, const glm::vec3& color, const glm::mat4& vp);
	static void drawVectorTrack(VectorTrack* track, float numCycle, float numSamplePerCycle, LoopType loop, float xoffset, float yoffset, const glm::vec3& color, const glm::mat4& vp);
	static void drawPose(const Pose& pose, const glm::vec3& color, const glm::mat4& vp);

protected:
	static std::unique_ptr<VertexArray> s_VAO;
	static std::unique_ptr<Buffer> s_VBO;
};