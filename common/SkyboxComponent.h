#pragma once
#include"RenderableComponent.h"
#include<glad/glad.h>

class Texture;
class VertexArray;
class Buffer;
struct SkyBox_t;


class SkyboxComponent : public RenderableComponent {
public:
	enum class Face {
		Positive_X = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		Negative_X = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		Positive_Y = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		Negative_Y = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		Positive_Z = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		Negative_Z = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
	};

public:
	SkyboxComponent();
	~SkyboxComponent();

	RTTI_DECLARATION(SkyboxComponent)
	
	bool initialize() override;
	void cleanUp();
	Component* copy() const override;
	void render(RenderContext* context) override;

	inline SkyBox_t makeSkyBox() const;

	bool load(const std::string& posX, const std::string& negX,
		const std::string& posY, const std::string& negY, 
		const std::string& posZ, const std::string& negZ);

	bool load(Face face, const std::string& image);

protected:
	std::unique_ptr<Texture> m_cubeMap;
	std::unique_ptr<VertexArray> m_cubeVAO;
	std::unique_ptr<Buffer> m_cubeVBO;
	std::unique_ptr<Buffer> m_cubeIBO;
};