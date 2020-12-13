#include"RenderTaskExecutor.h"
#include"VertexArray.h"
#include"Material.h"
#include"Buffer.h"
#include"ShaderProgram.h"
#include"ForwardRenderer.h"
#include"DeferredRenderer.h"
#include"Pose.h"
#include<glm/gtc/type_ptr.hpp>


#define MAX_NUM_BONE 156

static MaterialBlock s_materialBlock;

RenderTaskExecutor::RenderTaskExecutor(IRenderTechnique* rt) :m_renderer(rt) {

}


RenderTaskExecutor::~RenderTaskExecutor() {
	release();
}



DepthPassRenderTaskExecutor::DepthPassRenderTaskExecutor(IRenderTechnique* rt) :RenderTaskExecutor(rt) {

}

void DepthPassRenderTaskExecutor::executeMeshTask(const MeshRenderItem_t& renderTask, ShaderProgram* shader) {
	renderTask.vao->bind();
	
	if (shader->hasUniform("u_ModelMat")) {
		glm::mat4 m = renderTask.modelMatrix;
		shader->setUniformMat4v("u_ModelMat", &m[0][0]);
	}

	if (shader->hasSubroutineUniform(Shader::Type::VertexShader, "u_Transform")) {
		if (renderTask.boneCount <= 0) {
			shader->setSubroutineUniforms(Shader::Type::VertexShader, { {"u_Transform", "staticMesh"} });
		}
		else {
#ifdef _DEBUG
			ASSERT(renderTask.boneCount <= MAX_NUM_BONE);
#endif // _DEBUG
			int  numBone = MIN(renderTask.boneCount, MAX_NUM_BONE);
			shader->setUniformMat4v("u_SkinPose[0]", (float*)glm::value_ptr(renderTask.bonesTransform[0]), numBone);
			shader->setSubroutineUniforms(Shader::Type::VertexShader, { {"u_Transform", "skinMesh"} });
		}
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

	renderTask.vao->unbind();
}


UlitPassRenderTaskExecutror::UlitPassRenderTaskExecutror(IRenderTechnique* rt) :RenderTaskExecutor(rt) {

}

void UlitPassRenderTaskExecutror::executeMeshTask(const MeshRenderItem_t& renderTask, ShaderProgram* shader) {
	std::shared_ptr<Texture> strongDiffuseMap;
	renderTask.vao->bind();

	if (m_renderer->identifier() == ForwardRenderer::s_identifier) {
		if (shader->hasUniform("u_ModelMat")) {
			glm::mat4 m = renderTask.modelMatrix;
			shader->setUniformMat4v("u_ModelMat", &m[0][0]);
		}

		if (shader->hasSubroutineUniform(Shader::Type::VertexShader, "u_Transform")) {
			if (renderTask.boneCount <= 0) {
				shader->setSubroutineUniforms(Shader::Type::VertexShader, { {"u_Transform", "staticMesh"} });
			}
			else {
#ifdef _DEBUG
				ASSERT(renderTask.boneCount <= MAX_NUM_BONE);
#endif // _DEBUG
				int  numBone = MIN(renderTask.boneCount, MAX_NUM_BONE);
				shader->setUniformMat4v("u_SkinPose[0]", (float*)glm::value_ptr(renderTask.bonesTransform[0]), numBone);
				shader->setSubroutineUniforms(Shader::Type::VertexShader, { {"u_Transform", "skinMesh"} });
			}
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

	renderTask.vao->unbind();
	if (strongDiffuseMap)
		strongDiffuseMap->unbind();
}



LightPassRenderTaskExecuter::LightPassRenderTaskExecuter(IRenderTechnique* rt) : RenderTaskExecutor(rt) {

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


void LightPassRenderTaskExecuter::executeMeshTask(const MeshRenderItem_t& renderTask, ShaderProgram* shader) {
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

		if (shader->hasSubroutineUniform(Shader::Type::VertexShader, "u_Transform")) {
			if (renderTask.boneCount <= 0) {
				shader->setSubroutineUniforms(Shader::Type::VertexShader, { {"u_Transform", "staticMesh"} });
			}
			else {
#ifdef _DEBUG
				ASSERT(renderTask.boneCount <= MAX_NUM_BONE);
#endif // _DEBUG
				int  numBone = MIN(renderTask.boneCount, MAX_NUM_BONE);
				shader->setUniformMat4v("u_SkinPose[0]", (float*)glm::value_ptr(renderTask.bonesTransform[0]), numBone);
				shader->setSubroutineUniforms(Shader::Type::VertexShader, { {"u_Transform", "skinMesh"} });
			}
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

	renderTask.vao->unbind();
	if (strongDiffuseMap)
		strongDiffuseMap->unbind();
	if (strongNormalMap)
		strongNormalMap->unbind();
	if (strongSpecularMap)
		strongSpecularMap->unbind();
	if (strongEmissiveMap)
		strongEmissiveMap->unbind();
}


void LightPassRenderTaskExecuter::release() {
	m_materialUBO->release();
}



GeometryPassRenderTaskExecutor::GeometryPassRenderTaskExecutor(IRenderTechnique* rt) :RenderTaskExecutor(rt)
, m_materialUBO(nullptr) {

}

bool GeometryPassRenderTaskExecutor::initialize() {
	m_materialUBO.reset(new Buffer());
	m_materialUBO->bind(Buffer::Target::UniformBuffer);
	bool ok = m_materialUBO->loadData(nullptr, sizeof(MaterialBlock), Buffer::Usage::StaticDraw);
	m_materialUBO->unbind();

	return ok;
}

void GeometryPassRenderTaskExecutor::executeMeshTask(const MeshRenderItem_t& renderTask, ShaderProgram* shader) {
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

	if (shader->hasSubroutineUniform(Shader::Type::VertexShader, "u_Transform")) {
		if (renderTask.boneCount <= 0) {
			shader->setSubroutineUniforms(Shader::Type::VertexShader, { {"u_Transform", "staticMesh"} });
		}
		else {
#ifdef _DEBUG
			ASSERT(renderTask.boneCount <= MAX_NUM_BONE);
#endif // _DEBUG
			int  numBone = MIN(renderTask.boneCount, MAX_NUM_BONE);
			shader->setUniformMat4v("u_SkinPose[0]", (float*)glm::value_ptr(renderTask.bonesTransform[0]), numBone);
			shader->setSubroutineUniforms(Shader::Type::VertexShader, { {"u_Transform", "skinMesh"} });
		}
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

	renderTask.vao->unbind();
	if (strongDiffuseMap)
		strongDiffuseMap->unbind();
	if (strongNormalMap)
		strongNormalMap->unbind();
	if (strongSpecularMap)
		strongSpecularMap->unbind();
	if (strongEmissiveMap)
		strongEmissiveMap->unbind();
}

void GeometryPassRenderTaskExecutor::release() {
	m_materialUBO->release();
}


ShadowPassRenderTaskExecutor::ShadowPassRenderTaskExecutor(IRenderTechnique* rt) : RenderTaskExecutor(rt) {

}

void ShadowPassRenderTaskExecutor::executeMeshTask(const MeshRenderItem_t& renderTask, ShaderProgram* shader) {
	renderTask.vao->bind();
	
	// set model matrix
	if (shader->hasUniform("u_ModelMat")) {
		glm::mat4 m = renderTask.modelMatrix;
		shader->setUniformMat4v("u_ModelMat", &m[0][0]);
	}

	if (shader->hasSubroutineUniform(Shader::Type::VertexShader, "u_Transform")) {
		if (renderTask.boneCount <= 0) {
			shader->setSubroutineUniforms(Shader::Type::VertexShader, { {"u_Transform", "staticMesh"} });
		}
		else {
#ifdef _DEBUG
			ASSERT(renderTask.boneCount <= MAX_NUM_BONE);
#endif // _DEBUG
			int  numBone = MIN(renderTask.boneCount, MAX_NUM_BONE);
			shader->setUniformMat4v("u_SkinPose[0]", (float*)glm::value_ptr(renderTask.bonesTransform[0]), numBone);
			shader->setSubroutineUniforms(Shader::Type::VertexShader, { {"u_Transform", "skinMesh"} });
		}
	}

	if (renderTask.primitive == PrimitiveType::Triangle) {
		if (renderTask.indexCount > 0) {
			GLCALL(glDrawElements(GL_TRIANGLES, renderTask.indexCount, GL_UNSIGNED_INT, 0));
		}
		else {
			GLCALL(glDrawArrays(GL_TRIANGLES, 0, renderTask.vertexCount));
		}

	}
	//else if (renderTask.primitive == PrimitiveType::Line) {
	//	if (renderTask.indexCount > 0) {
	//		GLCALL(glDrawElements(GL_LINES, renderTask.indexCount, GL_UNSIGNED_INT, 0));
	//	}
	//	else {
	//		GLCALL(glDrawArrays(GL_LINES, 0, renderTask.vertexCount));
	//	}
	//}

	renderTask.vao->unbind();
}



AmbientPassRenderTaskExecutor::AmbientPassRenderTaskExecutor(IRenderTechnique* rt): RenderTaskExecutor(rt) {

}


void AmbientPassRenderTaskExecutor::executeMeshTask(const MeshRenderItem_t& task, ShaderProgram* shader) {
	std::shared_ptr<Texture> diffuseMap;
	std::shared_ptr<Texture> normalMap;
	task.vao->bind();

	// set model matrix
	if (shader->hasUniform("u_ModelMat")) {
		shader->setUniformMat4v("u_ModelMat", (float*)&task.modelMatrix[0][0]);
	}

	if (shader->hasSubroutineUniform(Shader::Type::VertexShader, "u_Transform")) {
		if (task.boneCount <= 0) {
			shader->setSubroutineUniforms(Shader::Type::VertexShader, { {"u_Transform", "staticMesh"} });
		}
		else {
#ifdef _DEBUG
			ASSERT(task.boneCount <= MAX_NUM_BONE);
#endif // _DEBUG
			int  numBone = MIN(task.boneCount, MAX_NUM_BONE);
			shader->setUniformMat4v("u_SkinPose[0]", (float*)glm::value_ptr(task.bonesTransform[0]), numBone);
			shader->setSubroutineUniforms(Shader::Type::VertexShader, { {"u_Transform", "skinMesh"} });
		}
	}

	if (shader->hasUniform("u_DiffuseMap")) {
		bool hasDiffuse = task.material->hasDiffuseTexture();
		shader->setUniform1("u_HasDiffuseMap", int(hasDiffuse));
		if (hasDiffuse) {
			diffuseMap = task.material->m_diffuseMap.lock();
			diffuseMap->bind(Texture::Unit::DiffuseMap);
			shader->setUniform1("u_DiffuseMap", int(Texture::Unit::DiffuseMap));
		}
	}

	if (shader->hasUniform("u_NormalMap")) {
		bool hasNormalMap = task.material->hasNormalTexture();
		shader->setUniform1("u_HasNormalMap", int(hasNormalMap));
		if (hasNormalMap) {
			normalMap = task.material->m_normalMap.lock();
			normalMap->bind(Texture::Unit::NormalMap);
			shader->setUniform1("u_NormalMap", int(Texture::Unit::NormalMap));
		}
	}

	if (task.primitive == PrimitiveType::Triangle) {
		if (task.indexCount > 0) {
			GLCALL(glDrawElements(GL_TRIANGLES, task.indexCount, GL_UNSIGNED_INT, 0));
		}
		else {
			GLCALL(glDrawArrays(GL_TRIANGLES, 0, task.vertexCount));
		}
	} else if (task.primitive == PrimitiveType::Line) {
		if (task.indexCount > 0) {
			GLCALL(glDrawElements(GL_LINES, task.indexCount, GL_UNSIGNED_INT, 0));
		}
		else {
			GLCALL(glDrawArrays(GL_LINES, 0, task.vertexCount));
		}
	}

	task.vao->unbind();
	if (diffuseMap)
		diffuseMap->unbind();
	if (normalMap)
		normalMap->unbind();
}