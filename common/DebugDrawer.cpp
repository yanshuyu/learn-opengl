#include"DebugDrawer.h"
#include"ShaderProgamMgr.h"
#include"Texture.h"
#include"Buffer.h"
#include"VertexArray.h"
#include"Util.h"
#include<glm/gtx/transform.hpp>
#include<algorithm>

std::unique_ptr<VertexArray> DebugDrawer::s_VAO = nullptr;
std::unique_ptr<Buffer> DebugDrawer::s_VBO = nullptr;

void DebugDrawer::setup() {
	s_VAO.reset(new VertexArray());
	s_VBO.reset(new Buffer());
}

void DebugDrawer::clenup() {
	s_VBO.release();
	s_VAO.release();
}

void DebugDrawer::drawTexture(Texture* texture, const glm::vec2& windowSz, const glm::vec4& rect) {
	auto shader = ShaderProgramManager::getInstance()->getProgram("TextureDebugViewer");
	if (!shader)
		shader = ShaderProgramManager::getInstance()->addProgram("res/shader/TextureDebugViewer.shader");

	ASSERT(shader);

	shader->bind();

	float pos_uv[] = { 
		0.f, 0.f, 0.f, 0.f,
		1.f, 0.f, 1.f, 0.f,
		1.f, 1.f, 1.f, 1.f,

		1.f, 1.f, 1.f, 1.f,
		0.f, 1.f, 0.f, 1.f,
		0.f, 0.f, 0.f, 0.f
	};

	s_VAO->bind();
	s_VBO->bind(Buffer::Target::VertexBuffer);
	s_VBO->loadData(pos_uv, 24 * sizeof(float), Buffer::Usage::StreamDraw);
	GLCALL(glEnableVertexAttribArray(0));
	GLCALL(glVertexAttribPointer(0, 2, GL_FLOAT, false, 4 * sizeof(float), (void*)0));
	GLCALL(glEnableVertexAttribArray(1));
	GLCALL(glVertexAttribPointer(1, 2, GL_FLOAT, false, 4 * sizeof(float), (void*)(2 * sizeof(float))));

	if (shader->hasUniform("u_MVP")) {
		glm::mat4 m(1.f);
		m = glm::translate(m, { rect.x, rect.y, 0.f });
		m = glm::scale(m, { rect.z, rect.w, 1.f });
		glm::mat4 p = glm::ortho(0.f, windowSz.x, 0.f, windowSz.y, 0.f, 10.f);
		m = p * m;
		shader->setUniformMat4v("u_MVP", &m[0][0]);
	}

	if (shader->hasUniform("u_texture")) {
		texture->bind(Texture::Unit::Defualt);
		shader->setUniform1("u_texture", int(Texture::Unit::Defualt));
	}

	GLCALL(glDrawArrays(GL_TRIANGLES, 0, 6));

	texture->unbind();
	shader->unbind();
	s_VBO->unbind();
	GLCALL(glDisableVertexAttribArray(0));
	GLCALL(glDisableVertexAttribArray(1));
	s_VAO->unbind();
}

void DebugDrawer::drawScalarTrack(ScalarTrack* track, float numCycle, float numSamplePerCycle, LoopType loop, float xoffset, float yoffset, float sampleWidth, const glm::vec3& color, const glm::mat4& vp) {
	if (!track->isValid())
		return;

	auto shader = ShaderProgramManager::getInstance()->getProgram("KeyFrameDebugViewer");
	if (!shader)
		shader = ShaderProgramManager::getInstance()->addProgram("res/shader/KeyFrameDebugViewer.shader");
	ASSERT(shader);

	shader->bind();

	float totalDur = track->getDuration() * numCycle + track->getStartTime();
	float totalSample = numSamplePerCycle * numCycle;
	float dt = totalDur / totalSample;

	std::vector<glm::vec3> points;
	points.reserve(totalSample);

	for (size_t i = 0; i < totalSample; i++) {
		float value = track->sample(i * dt, loop);
		glm::vec3 samplePoint(xoffset + i * sampleWidth, value + yoffset, 0.f);
		points.push_back(samplePoint);
	}	

	s_VAO->bind();
	s_VBO->bind(Buffer::Target::VertexBuffer);
	s_VBO->loadData(points.data(), sizeof(glm::vec3) * points.size(), Buffer::Usage::StreamDraw);
		
	GLCALL(glEnableVertexAttribArray(0));
	GLCALL(glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(glm::vec3), (void*)0));

	if (shader->hasUniform("u_MVP"))
		shader->setUniformMat4v("u_MVP", (float*)&vp[0][0]);
	if (shader->hasUniform("u_Color"))
		shader->setUniform4v("u_Color", (float*)&color[0]);

	GLCALL(glDrawArrays(GL_LINE_STRIP, 0, points.size()));

	shader->unbind();
	s_VBO->unbind();
	GLCALL(glDisableVertexAttribArray(0));
	s_VAO->unbind();
}


void DebugDrawer::drawVectorTrack(VectorTrack* track, float numCycle, float numSamplePerCycle, LoopType loop, float xoffset, float yoffset, const glm::vec3& color, const glm::mat4& vp) {
	if (!track->isValid())
		return;

	auto shader = ShaderProgramManager::getInstance()->getProgram("KeyFrameDebugViewer");
	if (!shader)
		shader = ShaderProgramManager::getInstance()->addProgram("res/shader/KeyFrameDebugViewer.shader");
	ASSERT(shader);

	shader->bind();

	float totalDur = track->getDuration() * numCycle + track->getStartTime();
	float totalSample = numSamplePerCycle * numCycle;
	float dt = totalDur / totalSample;

	std::vector<glm::vec3> points;
	points.reserve(totalSample);

	for (size_t i = 0; i < totalSample; i++) {
		glm::vec3 samplePoint = track->sample(i * dt, loop);
		samplePoint.x += xoffset;
		samplePoint.y += yoffset;
		points.push_back(samplePoint);
	}

	s_VAO->bind();
	s_VBO->bind(Buffer::Target::VertexBuffer);
	s_VBO->loadData(points.data(), sizeof(glm::vec3) * points.size(), Buffer::Usage::StreamDraw);

	GLCALL(glEnableVertexAttribArray(0));
	GLCALL(glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(glm::vec3), (void*)0));

	if (shader->hasUniform("u_MVP"))
		shader->setUniformMat4v("u_MVP", (float*)&vp[0][0]);
	if (shader->hasUniform("u_Color"))
		shader->setUniform4v("u_Color", (float*)&color[0]);

	GLCALL(glDrawArrays(GL_LINE_STRIP, 0, points.size()));

	shader->unbind();
	s_VBO->unbind();
	GLCALL(glDisableVertexAttribArray(0));
	s_VAO->unbind();
}