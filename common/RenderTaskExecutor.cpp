#include"RenderTaskExecutor.h"
#include"VertexArray.h"
#include"Material.h"
#include"Buffer.h"
#include"ShaderProgram.h"




RenderTaskExecutor::RenderTaskExecutor(RendererType rt) :m_rendererType(rt) {

}


RenderTaskExecutor::~RenderTaskExecutor() {
	release();
}



ZPassRenderTaskExecutor::ZPassRenderTaskExecutor(RendererType rt) :RenderTaskExecutor(rt) {

}

void ZPassRenderTaskExecutor::executeTask(const RenderTask_t& renderTask,
											const SceneRenderInfo_t& renderInfo,
											ShaderProgram* shader) {
	auto& camera = renderInfo.camera;
	renderTask.vao->bind();
	
	if (m_rendererType == RendererType::Forward) {
		// z-pass output depth test
		if (shader->hasUniform("u_near"))
			shader->setUniform1("u_near", camera.near);

		if (shader->hasUniform("u_far"))
			shader->setUniform1("u_far", camera.far);
	
		// set mvp matrix
		if (shader->hasUniform("u_MVP")) {
			glm::mat4 mvp = camera.projMatrix * camera.viewMatrix * renderTask.modelMatrix;
			shader->setUniformMat4v("u_MVP", &mvp[0][0]);
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
}


UlitPassRenderTaskExecutror::UlitPassRenderTaskExecutror(RendererType rt) :RenderTaskExecutor(rt) {

}

void UlitPassRenderTaskExecutror::executeTask(const RenderTask_t& renderTask,
												const SceneRenderInfo_t& renderInfo,
												ShaderProgram* shader) {
	auto& camera = renderInfo.camera;
	renderTask.vao->bind();

	if (m_rendererType == RendererType::Forward) {
		// set mvp matrix
		if (shader->hasUniform("u_MVP")) {
			glm::mat4 mvp = camera.projMatrix * camera.viewMatrix * renderTask.modelMatrix;
			shader->setUniformMat4v("u_MVP", &mvp[0][0]);
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
				renderTask.material->m_diffuseMap->bind(Texture::Unit::DiffuseMap, Texture::Target::Texture_2D);
				shader->setUniform1("u_diffuseMap", int(Texture::Unit::DiffuseMap));
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
}



LightPassRenderTaskExecuter::LightPassRenderTaskExecuter(RendererType rt) : RenderTaskExecutor(rt) {

}

bool LightPassRenderTaskExecuter::initialize() {
	m_materialUBO.reset(new Buffer());
	m_materialUBO->bind(Buffer::Target::UniformBuffer);
	m_materialUBO->loadData(nullptr, sizeof(MaterialBlock), Buffer::Usage::StaticDraw);
	m_materialUBO->unbind();

	return true;
}


void LightPassRenderTaskExecuter::executeTask(const RenderTask_t& renderTask, const SceneRenderInfo_t& renderInfo, ShaderProgram* shader) {
	if (m_rendererType == RendererType::Forward) {
		renderTask.vao->bind();

		// set matrixs
		if (shader->hasUniform("u_MVP")) {
			auto& camera = renderInfo.camera;
			glm::mat4 mvp = camera.projMatrix * camera.viewMatrix * renderTask.modelMatrix;
			shader->setUniformMat4v("u_MVP", &mvp[0][0]);
		}

		if (shader->hasUniform("u_ModelMat")) {
			glm::mat4 m = renderTask.modelMatrix;
			shader->setUniformMat4v("u_ModelMat", &m[0][0]);
		}

		// set materials
		if (shader->hasUniformBlock("MatrialBlock")) {
			MaterialBlock matBlock;
			matBlock.diffuseFactor = glm::vec4(renderTask.material->m_diffuseColor, renderTask.material->m_opacity);
			matBlock.specularFactor = glm::vec4(renderTask.material->m_specularColor, renderTask.material->m_shininess * Material::s_maxShininess);
			matBlock.emissiveColor = renderTask.material->m_emissiveColor;
			m_materialUBO->bind(Buffer::Target::UniformBuffer);
			m_materialUBO->loadSubData(&matBlock, 0, sizeof(matBlock));
			m_materialUBO->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::MaterialBlock));
			shader->bindUniformBlock("MatrialBlock", ShaderProgram::UniformBlockBindingPoint::MaterialBlock);
		}

		// set textures
		if (shader->hasUniform("u_diffuseMap")) {
			int hasDiffuseMap = renderTask.material->hasDiffuseTexture();
			shader->setUniform1("u_hasDiffuseMap", hasDiffuseMap);
			if (hasDiffuseMap) {
				renderTask.material->m_diffuseMap->bind(Texture::Unit::DiffuseMap, Texture::Target::Texture_2D);
				shader->setUniform1("u_diffuseMap", int(Texture::Unit::DiffuseMap));
			}
		}

		if (shader->hasUniform("u_emissiveMap")) {
			int hasEmissiveMap = renderTask.material->hasEmissiveTexture();
			shader->setUniform1("u_hasEmissiveMap", hasEmissiveMap);
			if (hasEmissiveMap) {
				renderTask.material->m_emissiveMap->bind(Texture::Unit::EmissiveMap, Texture::Target::Texture_2D);
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
}


void LightPassRenderTaskExecuter::release() {
	m_materialUBO->release();
}
