#pragma once
#include<common/GLApplication.h>
#include<memory>
#include<array>

class TextureCubeApp : public GLApplication {
public:
	TextureCubeApp(const std::string& title);

	bool initailize() override;

	void update(double dt) override;

	void render() override;

private:
	std::shared_ptr<ShaderProgram> m_shaderProgram;
	std::shared_ptr<Texture> m_abedoMap;
	std::shared_ptr<Buffer> m_vbo;
	std::shared_ptr<Buffer> m_ibo;
	std::shared_ptr<VertexArray> m_vao;
};