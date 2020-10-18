#include"RenderTaskExecutor.h"
#include"VertexArray.h"
#include"Material.h"
#include"Buffer.h"
#include"ShaderProgram.h"
#include"ForwardRenderer.h"
#include"DeferredRenderer.h"


static MaterialBlock s_materialBlock;


RenderTaskExecutor::RenderTaskExecutor(RenderTechnique* rt) :m_renderer(rt) {

}


RenderTaskExecutor::~RenderTaskExecutor() {
	release();
}



DepthPassRenderTaskExecutor::DepthPassRenderTaskExecutor(RenderTechnique* rt) :RenderTaskExecutor(rt) {

}

void DepthPassRenderTaskExecutor::executeTask(const RenderTask_t& renderTask, ShaderProgram* shader) {
	renderTask.vao->bind();
	
	if (shader->hasUniform("u_ModelMat")) {
		glm::mat4 m = renderTask.modelMatrix;
		shader->setUniformMat4v("u_ModelMat", &m[0][0]);
	}

	if (renderTask.primitive == PrimitiveType::Triangle) {
		if (renderTask.indexCount > 0) {
			GLCALL(glDrawElements(GL_TRIANGLES, renderTask.indexCount, GL_UNSIGNED_INT, 0));
		} else {
			GLCALL(glDrawArrays(GL_TRIANGLES, 0, renderTask.vertexCount));
		}
		
	} else if (renderTask.primitive == PrimitiveType::Line) {
		if (renderTask.indexCount > 0) {
			GLCALL(glDrawElements(GL_LINES, renderTask.indexCount, GL_UNSIGNED_INT, 0));
		} else {
			GLCALL(glDrawArrays(GL_LINES, 0, renderTask.vertexCount));
		}
	}

}


UlitPassRenderTaskExecutror::UlitPassRenderTaskExecutror(RenderTechnique* rt) :RenderTaskExecutor(rt) {

}

void UlitPassRenderTaskExecutror::executeTask(const RenderTask_t& renderTask, ShaderProgram* shader) {
	std::shared_ptr<Texture> strongDiffuseMap;

	if (m_renderer->identifier() == ForwardRenderer::s_identifier) {
		if (shader->hasUniform("u_ModelMat")) {
			glm::mat4 m = renderTask.modelMatrix;
			shader->setUniformMat4v("u_ModelMat", &m[0][0]);
		}

		// set material
		if (shader->hasUniform("u_diffuseColor")) {
			shader->setUniform4("u_diffuseColor",
				renderTask.material->m_diffuseColor.r,
				renderTask.material->m_diffuseColor.g,
				renderTask.material->m_diffuseColor.b,
				renderTask.material->m_opacity);
		}
		// set textures
		if (shader->hasUniform("u_diffuseMap")) {
			int hasTexture = renderTask.material->hasDiffuseTexture() ? 1 : 0;
			shader->setUniform1("u_hasDiffuseMap", hasTexture);
			if (hasTexture) {
				strongDiffuseMap = renderTask.material->m_diffuseMap.lock();
				strongDiffuseMap->bind(Texture::Unit::DiffuseMap, Texture::Target::Texture_2D);
				shader->setUniform1("u_diffuseMap", int(Texture::Unit::DiffuseMap));
			}
		}

	} else if (m_renderer->identifier() == DeferredRenderer::s_identifier) {
		auto renderer = static_cast<DeferredRenderer*>(m_renderer);
		auto shader = renderer->m_activeShader;

		if (shader->hasUniform("u_diffuse")) {
			renderer->m_diffuseBuffer->bind(Texture::Unit::DiffuseMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_diffuse", int(Texture::Unit::DiffuseMap));
		}

		if (shader->hasUniform("u_emissive")) {
			renderer->m_emissiveBuffer->bind(Texture::Unit::EmissiveMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_emissive", int(Texture::Unit::EmissiveMap));
		}

	} else {
		ASSERT(false);
	}

	renderTask.vao->bind();

	// draw
	if (renderTask.primitive == PrimitiveType::Triangle) {
		if (renderTask.indexCount > 0) {
			GLCALL(glDrawElements(GL_TRIANGLES, renderTask.indexCount, GL_UNSIGNED_INT, 0));
		}
		else {
			GLCALL(glDrawArrays(GL_TRIANGLES, 0, renderTask.vertexCount));
		}

	}
	else if (renderTask.primitive == PrimitiveType::Line) {
		if (renderTask.indexCount > 0) {
			GLCALL(glDrawElements(GL_LINES, renderTask.indexCount, GL_UNSIGNED_INT, 0));
		}
		else {
			GLCALL(glDrawArrays(GL_LINES, 0, renderTask.vertexCount));
		}
	}
}



LightPassRenderTaskExecuter::LightPassRenderTaskExecuter(RenderTechnique* rt) : RenderTaskExecutor(rt) {

}

bool LightPassRenderTaskExecuter::initialize() {
	if (m_renderer->identifier() == DeferredRenderer::s_identifier)
		return true;

	m_materialUBO.reset(new Buffer());
	m_materialUBO->bind(Buffer::Target::UniformBuffer);
	bool ok = m_materialUBO->loadData(nullptr, sizeof(MaterialBlock), Buffer::Usage::StaticDraw);
	m_materialUBO->unbind();

	return ok;
}


void LightPassRenderTaskExecuter::executeTask(const RenderTask_t& renderTask, ShaderProgram* shader) {
	renderTask.vao->bind();
	std::shared_ptr<Texture> strongDiffuseMap;
	std::shared_ptr<Texture> strongNormalMap;
	std::shared_ptr<Texture> strongSpecularMap;
	std::shared_ptr<Texture> strongEmissiveMap;

	if (m_renderer->identifier() == ForwardRenderer::s_identifier) {
		if (shader->hasUniform("u_ModelMat")) {
			glm::mat4 m = renderTask.modelMatrix;
			shader->setUniformMat4v("u_ModelMat", &m[0][0]);
		}

		// set materials
		if (shader->hasUniformBlock("MatrialBlock")) {
			s_materialBlock.diffuseFactor = glm::vec4(renderTask.material->m_diffuseColor, renderTask.material->m_opacity);
			s_materialBlock.specularFactor = glm::vec4(renderTask.material->m_specularColor, renderTask.material->m_shininess * Material::s_maxShininess);
			s_materialBlock.emissiveColor = renderTask.material->m_emissiveColor;
			m_materialUBO->bind(Buffer::Target::UniformBuffer);
			m_materialUBO->loadSubData(&s_materialBlock, 0, sizeof(s_materialBlock));
			m_materialUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::MaterialBlock));
			shader->bindUniformBlock("MatrialBlock", ShaderProgram::UniformBlockBindingPoint::MaterialBlock);
		}

		// set textures
		if (shader->hasUniform("u_diffuseMap")) {
			int hasDiffuseMap = renderTask.material->hasDiffuseTexture();
			shader->setUniform1("u_hasDiffuseMap", hasDiffuseMap);
			if (hasDiffuseMap) {
				strongDiffuseMap = renderTask.material->m_diffuseMap.lock();
				strongDiffuseMap->bind(Texture::Unit::DiffuseMap, Texture::Target::Texture_2D);
				shader->setUniform1("u_diffuseMap", int(Texture::Unit::DiffuseMap));
			}
		}

		if (shader->hasUniform("u_normalMap")) {
			int hasNormalMap = renderTask.material->hasNormalTexture();
			shader->setUniform1("u_hasNormalMap", hasNormalMap);
			if (hasNormalMap) {
				strongNormalMap = renderTask.material->m_normalMap.lock();
				strongNormalMap->bind(Texture::Unit::NormalMap, Texture::Target::Texture_2D);
				shader->setUniform1("u_normalMap", int(Texture::Unit::NormalMap));
			}
		}

		if (shader->hasUniform("u_specularMap")) {
			int hasSpecMap = renderTask.material->hasSpecularTexture();
			shader->setUniform1("u_hasSpecularMap", hasSpecMap);
			if (hasSpecMap) {
				strongSpecularMap = renderTask.material->m_specularMap.lock();
				strongSpecularMap->bind(Texture::Unit::SpecularMap, Texture::Target::Texture_2D);
				shader->setUniform1("u_specularMap", int(Texture::Unit::SpecularMap));
			}
		}

		if (shader->hasUniform("u_emissiveMap")) {
			int hasEmissiveMap = renderTask.material->hasEmissiveTexture();
			shader->setUniform1("u_hasEmissiveMap", hasEmissiveMap);
			if (hasEmissiveMap) {
				strongEmissiveMap = renderTask.material->m_emissiveMap.lock();
				strongEmissiveMap->bind(Texture::Unit::EmissiveMap, Texture::Target::Texture_2D);
				shader->setUniform1("u_emissiveMap", int(Texture::Unit::EmissiveMap));
			}
		}
	}

	// derferred renderer need to do nothing, just commit draw command

	// draw
	if (renderTask.primitive == PrimitiveType::Triangle) {
		if (renderTask.indexCount > 0) {
			GLCALL(glDrawElements(GL_TRIANGLES, renderTask.indexCount, GL_UNSIGNED_INT, 0));
		}
		else {
			GLCALL(glDrawArrays(GL_TRIANGLES, 0, renderTask.vertexCount));
		}

	}
	else if (renderTask.primitive == PrimitiveType::Line) {
		if (renderTask.indexCount > 0) {
			GLCALL(glDrawElements(GL_LINES, renderTask.indexCount, GL_UNSIGNED_INT, 0));
		}
		else {
			GLCALL(glDrawArrays(GL_LINES, 0, renderTask.vertexCount));
		}
	}
}


void LightPassRenderTaskExecuter::release() {
	m_materialUBO->release();
}



GeometryPassRenderTaskExecutor::GeometryPassRenderTaskExecutor(RenderTechnique* rt) :RenderTaskExecutor(rt)
, m_materialUBO(nullptr) {

}

bool GeometryPassRenderTaskExecutor::initialize() {
	m_materialUBO.reset(new Buffer());
	m_materialUBO->bind(Buffer::Target::UniformBuffer);
	bool ok = m_materialUBO->loadData(nullptr, sizeof(MaterialBlock), Buffer::Usage::StaticDraw);
	m_materialUBO->unbind();

	return ok;
}

void GeometryPassRenderTaskExecutor::executeTask(const RenderTask_t& renderTask, ShaderProgram* shader) {
	auto renderer = static_cast<DeferredRenderer*>(m_renderer);
	std::shared_ptr<Texture> strongDiffuseMap;
	std::shared_ptr<Texture> strongNormalMap;
	std::shared_ptr<Texture> strongSpecularMap;
	std::shared_ptr<Texture> strongEmissiveMap;

	renderTask.vao->bind();

	// set matrixs
	if (shader->hasUniform("u_ModelMat")) {
		glm::mat4 m = renderTask.modelMatrix;
		shader->setUniformMat4v("u_ModelMat", &m[0][0]);
	}

	// set materials
	if (shader->hasUniformBlock("MatrialBlock")) {
		s_materialBlock.diffuseFactor = glm::vec4(renderTask.material->m_diffuseColor, renderTask.material->m_opacity);
		s_materialBlock.specularFactor = glm::vec4(renderTask.material->m_specularColor, renderTask.material->m_shininess);
		s_materialBlock.emissiveColor = renderTask.material->m_emissiveColor;
		m_materialUBO->bind(Buffer::Target::UniformBuffer);
		m_materialUBO->loadSubData(&s_materialBlock, 0, sizeof(s_materialBlock));
		m_materialUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::MaterialBlock));
		shader->bindUniformBlock("MatrialBlock", ShaderProgram::UniformBlockBindingPoint::MaterialBlock);
	}

	// set textures
	if (shader->hasUniform("u_diffuseMap")) {
		int hasDiffuseMap = renderTask.material->hasDiffuseTexture();
		shader->setUniform1("u_hasDiffuseMap", hasDiffuseMap);
		if (hasDiffuseMap) {
			strongDiffuseMap = renderTask.material->m_diffuseMap.lock();
			strongDiffuseMap->bind(Texture::Unit::DiffuseMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_diffuseMap", int(Texture::Unit::DiffuseMap));
		}
	}

	if (shader->hasUniform("u_normalMap")) {
		int hasNormalMap = renderTask.material->hasNormalTexture();
		shader->setUniform1("u_hasNormalMap", hasNormalMap);
		if (hasNormalMap) {
			strongNormalMap = renderTask.material->m_normalMap.lock();
			strongNormalMap->bind(Texture::Unit::NormalMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_normalMap", int(Texture::Unit::NormalMap));
		}
	}

	if (shader->hasUniform("u_specularMap")) {
		int hasSpecMap = renderTask.material->hasSpecularTexture();
		shader->setUniform1("u_hasSpecularMap", hasSpecMap);
		if (hasSpecMap) {
			strongSpecularMap = renderTask.material->m_specularMap.lock();
			strongSpecularMap->bind(Texture::Unit::SpecularMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_specularMap", int(Texture::Unit::SpecularMap));
		}
	}

	if (shader->hasUniform("u_emissiveMap")) {
		int hasEmissiveMap = renderTask.material->hasEmissiveTexture();
		shader->setUniform1("u_hasEmissiveMap", hasEmissiveMap);
		if (hasEmissiveMap) {
			strongEmissiveMap = renderTask.material->m_emissiveMap.lock();
			strongEmissiveMap->bind(Texture::Unit::EmissiveMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_emissiveMap", int(Texture::Unit::EmissiveMap));
		}
	}

	// draw
	if (renderTask.primitive == PrimitiveType::Triangle) {
		if (renderTask.indexCount > 0) {
			GLCALL(glDrawElements(GL_TRIANGLES, renderTask.indexCount, GL_UNSIGNED_INT, 0));
		}
		else {
			GLCALL(glDrawArrays(GL_TRIANGLES, 0, renderTask.vertexCount));
		}

	}
	else if (renderTask.primitive == PrimitiveType::Line) {
		if (renderTask.indexCount > 0) {
			GLCALL(glDrawElements(GL_LINES, renderTask.indexCount, GL_UNSIGNED_INT, 0));
		}
		else {
			GLCALL(glDrawArrays(GL_LINES, 0, renderTask.vertexCount));
		}
	}
	
}

void GeometryPassRenderTaskExecutor::release() {
	m_materialUBO->release();
}


ShadowPassRenderTaskExecutor::ShadowPassRenderTaskExecutor(RenderTechnique* rt) : RenderTaskExecutor(rt) {

}

void ShadowPassRenderTaskExecutor::executeTask(const RenderTask_t& renderTask, ShaderProgram* shader) {
	renderTask.vao->bind();
	
	// set model matrix
	if (shader->hasUniform("u_ModelMat")) {
		glm::mat4 m = renderTask.modelMatrix;
		shader->setUniformMat4v("u_ModelMat", &m[0][0]);
	}

	if (renderTask.primitive == PrimitiveType::Triangle) {
		if (renderTask.indexCount > 0) {
			GLCALL(glDrawElements(GL_TRIANGLES, renderTask.indexCount, GL_UNSIGNED_INT, 0));
		}
		else {
			GLCALL(glDrawArrays(GL_TRIANGLES, 0, renderTask.vertexCount));
		}

	}
	else if (renderTask.primitive == PrimitiveType::Line) {
		if (renderTask.indexCount > 0) {
			GLCALL(glDrawElements(GL_LINES, renderTask.indexCount, GL_UNSIGNED_INT, 0));
		}
		else {
			GLCALL(glDrawArrays(GL_LINES, 0, renderTask.vertexCount));
		}
	}
	
}