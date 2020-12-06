#include"SkyboxComponent.h"
#include"VertexArray.h"
#include"Buffer.h"
#include"Texture.h"
#include"Renderer.h"
#include"TextureMgr.h"
#include"Util.h"
#include<stb_image/stb_image.h>


RTTI_IMPLEMENTATION(SkyboxComponent)

SkyboxComponent::SkyboxComponent() : RenderableComponent()
, m_cubeVAO()
, m_cubeVBO()
, m_cubeIBO()
, m_cubeMap() {
	m_cubeVAO.reset(new VertexArray);
	m_cubeVBO.reset(new Buffer);
	m_cubeIBO.reset(new Buffer);
	m_cubeMap.reset(new Texture);
}

SkyboxComponent::~SkyboxComponent() {
	cleanUp();
}

bool SkyboxComponent::initialize() {
	std::array<glm::vec3, 8> vertices;
	vertices[0] = { 0.5, 0.5, 0.5 };
	vertices[1] = { 0.5, -0.5, 0.5 };
	vertices[2] = { -0.5, -0.5, 0.5 };
	vertices[3] = { -0.5, 0.5, 0.5 };

	vertices[4] = { -0.5, -0.5, -0.5 };
	vertices[5] = { 0.5, -0.5, -0.5 };
	vertices[6] = { 0.5, 0.5, -0.5 };
	vertices[7] = { -0.5, 0.5, -0.5 };

	std::array<unsigned int, 36> indices;
	// +z - z
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 0;
	indices[4] = 2;
	indices[5] = 3;

	indices[6] = 4;
	indices[7] = 5;
	indices[8] = 6;
	indices[9] = 4;
	indices[10] = 6;
	indices[11] = 7;

	// +x -x
	indices[12] = 1;
	indices[13] = 0;
	indices[14] = 6;
	indices[15] = 1;
	indices[16] = 6;
	indices[17] = 5;

	indices[18] = 4;
	indices[19] = 7;
	indices[20] = 3;
	indices[21] = 4;
	indices[22] = 3;
	indices[23] = 2;

	// +y -y
	indices[24] = 3;
	indices[25] = 7;
	indices[26] = 6;
	indices[27] = 3;
	indices[28] = 6;
	indices[29] = 0;

	indices[30] = 5;
	indices[31] = 4;
	indices[32] = 2;
	indices[33] = 5;
	indices[34] = 2;
	indices[35] = 1;

	m_cubeVBO->bind(Buffer::Target::VertexBuffer);
	if (!m_cubeVBO->loadData(vertices.data(), vertices.size() * sizeof(glm::vec3), Buffer::Usage::StaticDraw, vertices.size())) {
		m_cubeVBO->unbind();
		cleanUp();
		return false;
	}
	m_cubeVBO->unbind();

	m_cubeIBO->bind(Buffer::Target::IndexBuffer);
	if (!m_cubeIBO->loadData(indices.data(), indices.size() * sizeof(unsigned int), Buffer::Usage::StaticDraw, indices.size())) {
		m_cubeIBO->unbind();
		cleanUp();
		return false;
	}
	m_cubeIBO->unbind();

	VertexLayoutDescription layoutDesc;
	layoutDesc.pushAttribute(VertexLayoutDescription::AttributeElementType::FLOAT, 3, 0);

	m_cubeVAO->bind();
	m_cubeVBO->bind(Buffer::Target::VertexBuffer);
	m_cubeIBO->bind(Buffer::Target::IndexBuffer);
	m_cubeVAO->storeVertexLayout(layoutDesc);
	m_cubeVAO->unbind();
	m_cubeVBO->unbind();
	m_cubeIBO->unbind();

	return true;
}


Component* SkyboxComponent::copy() const {
	return nullptr;
};


void SkyboxComponent::render(RenderContext* context) {
	context->getRenderer()->submitSkyBox(makeSkyBox());
}

bool SkyboxComponent::load(const std::string& posX, const std::string& negX,
	const std::string& posY, const std::string& negY,
	const std::string& posZ, const std::string& negZ) {
	m_cubeMap->bind(Texture::Unit::Defualt, Texture::Target::Texture_CubeMap);
	auto texMgr = TextureManager::getInstance();
	bool ok = m_cubeMap->loadCubeMapFromFiles(texMgr->getResourcePath( ExtractFileNameFromPath(posX)), 
		texMgr->getResourcePath(ExtractFileNameFromPath(negX)), 
		texMgr->getResourcePath(ExtractFileNameFromPath(posY)), 
		texMgr->getResourcePath(ExtractFileNameFromPath(negY)), 
		texMgr->getResourcePath(ExtractFileNameFromPath(posZ)),
		texMgr->getResourcePath(ExtractFileNameFromPath(negZ)));
	m_cubeMap->unbind();

	return ok;
}


bool SkyboxComponent::load(Face face, const std::string& image) {
	int channelCnt = 0;
	int w, h;
	stbi_set_flip_vertically_on_load(true);
	auto data = stbi_load(TextureManager::getInstance()->getResourcePath(ExtractFileNameFromPath(image)).c_str(), &w, &h, &channelCnt, 0);

	if (!data)
		return false;

	m_cubeMap->bind(Texture::Unit::Defualt, Texture::Target::Texture_CubeMap);
	Texture::Format fmt = m_cubeMap->getGenericFormat(channelCnt);
	GLCALL(glTexImage2D(int(face), 0, int(fmt), w, h, 0, int(fmt), GL_UNSIGNED_BYTE, data));
	m_cubeMap->unbind();

	return true;
}


void SkyboxComponent::cleanUp() {
	m_cubeVAO.release();
	m_cubeVBO.release();
	m_cubeIBO.release();
	m_cubeMap.release();
}


SkyBox_t SkyboxComponent::makeSkyBox() const {
	return SkyBox_t(m_cubeVAO.get(), m_cubeMap.get());
}