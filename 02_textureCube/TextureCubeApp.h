#pragma once
#include<common/GLApplication.h>
#include<memory>
#include<array>
#include<glm/glm.hpp>

class TextureCubeApp : public GLApplication {
	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 uv;

		Vertex(const glm::vec3& _position, const glm::vec3& _color, const glm::vec2& _uv) {
			position = _position;
			color = _color;
			uv = _uv;
		}
	};
public:
	TextureCubeApp(const std::string& title);

	bool initailize() override;

	void update(double dt) override;

	void render() override;

	void onWindowResized(int w, int h) override;

private:
	std::shared_ptr<ShaderProgram> m_shaderProgram;
	std::shared_ptr<Texture> m_abedoMap;
	
	std::unique_ptr<Buffer> m_vbo_cube;
	std::unique_ptr<Buffer> m_ibo_cube;
	std::unique_ptr<VertexArray> m_vao_cube;

	std::unique_ptr<Buffer> m_vbo_qaud;
	std::unique_ptr<Buffer> m_ibo_qaud;
	std::unique_ptr<VertexArray> m_vao_qaud;

	glm::mat4 m_modelMat;
	glm::mat4 m_viewMat;
	glm::mat4 m_projMat;
	glm::mat4 m_orthMat;

	std::unique_ptr<SceneObject> m_camera;
};