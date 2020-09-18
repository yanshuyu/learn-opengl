#include"RenderTaskExecutor.h"
#include"VertexArray.h"
#include"Material.h"

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
		if (shader->hasUniform("u_mvp")) {
			glm::mat4 mvp = camera.projMatrix * camera.viewMatrix * renderTask.modelMatrix;
			shader->setUniformMat4v("u_mvp", &mvp[0][0]);
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
		if (shader->hasUniform("u_mvp")) {
			glm::mat4 mvp = camera.projMatrix * camera.viewMatrix * renderTask.modelMatrix;
			shader->setUniformMat4v("u_mvp", &mvp[0][0]);
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
