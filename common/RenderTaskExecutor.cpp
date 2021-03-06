#include"RenderTaskExecutor.h"
#include"VertexArray.h"
#include"PhongMaterial.h"
#include"PBRMaterial.h"
#include"Buffer.h"
#include"ShaderProgram.h"
#include"ForwardRenderer.h"
#include"DeferredRenderer.h"
#include"Pose.h"
#include"Renderer.h"
#include<glm/gtc/type_ptr.hpp>

#define MAX_NUM_BONES 256
static std::unique_ptr<Buffer> s_SkinPoseBlockBuf;
static std::unique_ptr<Buffer> s_MaterialBlockBuf;
static MaterialBlock s_MaterialBlock;

bool RENDER_TASK_EXECUTOR_INIT() {
	s_SkinPoseBlockBuf.reset(new Buffer());
	s_SkinPoseBlockBuf->bind(Buffer::Target::UniformBuffer);
	s_SkinPoseBlockBuf->loadData(nullptr, sizeof(glm::mat4) * MAX_NUM_BONES, Buffer::Usage::DynamicDraw);
	s_SkinPoseBlockBuf->unbind();

	s_MaterialBlockBuf.reset(new Buffer());
	s_MaterialBlockBuf->bind(Buffer::Target::UniformBuffer);
	s_MaterialBlockBuf->loadData(nullptr, sizeof(MaterialBlock), Buffer::Usage::DynamicDraw);
	s_MaterialBlockBuf->unbind();

	return true;
}


void RENDER_TASK_EXECUTOR_DEINIT() {
	s_SkinPoseBlockBuf.release();
	s_MaterialBlockBuf.release();
}


RenderTaskExecutor::RenderTaskExecutor(IRenderTechnique* rt) :m_renderer(rt) {

}


RenderTaskExecutor::~RenderTaskExecutor() {
	release();
}



DepthPassRenderTaskExecutor::DepthPassRenderTaskExecutor(IRenderTechnique* rt) :RenderTaskExecutor(rt) {

}

void DepthPassRenderTaskExecutor::executeMeshTask(const MeshRenderItem_t& renderTask, ShaderProgram* shader) {	
	if (shader->hasUniform("u_ModelMat")) {
		glm::mat4 m = renderTask.modelMatrix;
		shader->setUniformMat4v("u_ModelMat", &m[0][0]);
	}

	if (shader->hasSubroutineUniform(Shader::Type::VertexShader, "u_Transform")) {
		if (renderTask.boneCount <= 0) {
			shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "staticMesh");
		}
		else {
#ifdef _DEBUG
			ASSERT(renderTask.boneCount <= MAX_NUM_BONES);
#endif // _DEBUG
			int  numBone = MIN(renderTask.boneCount, MAX_NUM_BONES);
			shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "skinMesh");
			s_SkinPoseBlockBuf->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::SkinPoseBlock));
			s_SkinPoseBlockBuf->loadSubData(renderTask.bonesTransform, 0, sizeof(glm::mat4) * numBone);
			shader->bindUniformBlock("SkinPoseBlock", ShaderProgram::UniformBlockBindingPoint::SkinPoseBlock);
		}
	}

	shader->bindSubroutineUniforms();
	
	m_renderer->getRenderer()->executeDrawCommand(renderTask.vao, renderTask.primitive, renderTask.vertexCount, renderTask.indexCount);

	if (renderTask.boneCount > 0) s_SkinPoseBlockBuf->unbind();
}


UlitPassRenderTaskExecutror::UlitPassRenderTaskExecutror(IRenderTechnique* rt) :RenderTaskExecutor(rt) {

}

void UlitPassRenderTaskExecutror::executeMeshTask(const MeshRenderItem_t& renderTask, ShaderProgram* shader) {
	/*
	std::shared_ptr<Texture> strongDiffuseMap;
	renderTask.vao->bind();
	
	if (m_renderer->identifier() == ForwardRenderer::s_identifier) {
		if (shader->hasUniform("u_ModelMat")) {
			glm::mat4 m = renderTask.modelMatrix;
			shader->setUniformMat4v("u_ModelMat", &m[0][0]);
		}

		if (shader->hasSubroutineUniform(Shader::Type::VertexShader, "u_Transform")) {
			if (renderTask.boneCount <= 0) {
				shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "staticMesh");
			}
			else {
#ifdef _DEBUG
				ASSERT(renderTask.boneCount <= MAX_NUM_BONES);
#endif // _DEBUG
				int  numBone = MIN(renderTask.boneCount, MAX_NUM_BONES);
				shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "skinMesh");
				s_SkinPoseBlockBuf->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::SkinPoseBlock));
				s_SkinPoseBlockBuf->loadSubData(renderTask.bonesTransform, 0, sizeof(glm::mat4) * numBone);
				shader->bindUniformBlock("SkinPoseBlock", ShaderProgram::UniformBlockBindingPoint::SkinPoseBlock);
			}
		}

		
		// set material
		if (auto mtl = renderTask.material->asType<PhongMaterial>()) {
			if (shader->hasUniform("u_diffuseColor")) {
				shader->setUniform4("u_diffuseColor",
					mtl->m_mainColor.r,
					mtl->m_mainColor.g,
					mtl->m_mainColor.b,
					mtl->m_opacity);
			}
			// set textures
			if (shader->hasUniform("u_diffuseMap")) {
				int hasTexture = mtl->hasAlbedoMap() ? 1 : 0;
				shader->setUniform1("u_hasDiffuseMap", hasTexture);
				if (hasTexture) {
					strongDiffuseMap = mtl->m_albedoMap.lock();
					strongDiffuseMap->bind(Texture::Unit::DiffuseMap, Texture::Target::Texture_2D);
					shader->setUniform1("u_diffuseMap", int(Texture::Unit::DiffuseMap));
				}
			}
		}
	
	} 

	shader->bindSubroutineUniforms();
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
	if (strongDiffuseMap) strongDiffuseMap->unbind();
	if (renderTask.boneCount > 0) s_SkinPoseBlockBuf->unbind();
	*/
}



LightPassRenderTaskExecuter::LightPassRenderTaskExecuter(IRenderTechnique* rt) : RenderTaskExecutor(rt) {

}


void LightPassRenderTaskExecuter::executeMeshTask(const MeshRenderItem_t& renderTask, ShaderProgram* shader) {
	std::shared_ptr<Texture> strongDiffuseMap;
	std::shared_ptr<Texture> strongNormalMap;
	std::shared_ptr<Texture> strongSpecularMap;
	std::shared_ptr<Texture> strongMetallicMap;
	std::shared_ptr<Texture> strongroughnessMap;

	if (shader->hasUniform("u_ModelMat")) {
		shader->setUniformMat4v("u_ModelMat", &renderTask.modelMatrix[0][0]);
	}

	if (shader->hasSubroutineUniform(Shader::Type::VertexShader, "u_Transform")) {
		if (renderTask.boneCount <= 0) {
			shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "staticMesh");
		}
		else {
#ifdef _DEBUG
			ASSERT(renderTask.boneCount <= MAX_NUM_BONES);
#endif // _DEBUG
			int  numBone = MIN(renderTask.boneCount, MAX_NUM_BONES);
			shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "skinMesh");
			s_SkinPoseBlockBuf->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::SkinPoseBlock));
			s_SkinPoseBlockBuf->loadSubData(renderTask.bonesTransform, 0, sizeof(glm::mat4) * numBone);
			shader->bindUniformBlock("SkinPoseBlock", ShaderProgram::UniformBlockBindingPoint::SkinPoseBlock);
		}
	}
	
	if (auto mtl = renderTask.material->asType<PhongMaterial>()) {
		shader->setSubroutineUniform(Shader::Type::FragmentShader, "u_ShadingMode", "PhongShading");

		s_MaterialBlock.albedo = glm::vec4(mtl->m_mainColor, mtl->m_opacity);
		s_MaterialBlock.specular = glm::vec4(mtl->m_specularColor, mtl->m_shininess * PhongMaterial::s_maxShininess);
		s_MaterialBlock.emissive = mtl->m_emissive;
		s_MaterialBlock.metalness = 0.f;
		s_MaterialBlock.roughness = 0.f;

		// set textures
		int hasAlbedoMap = mtl->hasAlbedoMap();
		if (hasAlbedoMap) {
			strongDiffuseMap = mtl->m_albedoMap.lock();
			strongDiffuseMap->bindToTextureUnit(Texture::Unit::DiffuseMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_AlbedoMap", int(Texture::Unit::DiffuseMap));
		}
	
		int hasNormalMap = mtl->hasNormalMap();
		if (hasNormalMap) {
			strongNormalMap = mtl->m_normalMap.lock();
			strongNormalMap->bindToTextureUnit(Texture::Unit::NormalMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_NormalMap", int(Texture::Unit::NormalMap));
		}	

		int hasSpecMap = mtl->hasSpecularMap();
		if (hasSpecMap) {
			strongSpecularMap = mtl->m_specularMap.lock();
			strongSpecularMap->bindToTextureUnit(Texture::Unit::SpecularMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_SpecularMap", int(Texture::Unit::SpecularMap));
		}

		shader->setUniform4("u_HasANRMMap", hasAlbedoMap, hasNormalMap, hasSpecMap, 0);
	}

	if (auto mtl = renderTask.material->asType<PBRMaterial>()) {
		shader->setSubroutineUniform(Shader::Type::FragmentShader, "u_ShadingMode", "PBRShading");

		s_MaterialBlock.albedo = glm::vec4(mtl->m_mainColor, mtl->m_opacity);
		s_MaterialBlock.specular = glm::vec4(0.f);
		s_MaterialBlock.emissive = mtl->m_emissive;
		s_MaterialBlock.metalness = mtl->m_metallic;
		s_MaterialBlock.roughness = mtl->m_roughness;

		// set textures
		int hasAlbedoMap = mtl->hasAlbedoMap();
		if (hasAlbedoMap) {
			strongDiffuseMap = mtl->m_albedoMap.lock();
			strongDiffuseMap->bindToTextureUnit(Texture::Unit::DiffuseMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_AlbedoMap", int(Texture::Unit::DiffuseMap));
		}

		int hasNormalMap = mtl->hasNormalMap();
		if (hasNormalMap) {
			strongNormalMap = mtl->m_normalMap.lock();
			strongNormalMap->bindToTextureUnit(Texture::Unit::NormalMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_NormalMap", int(Texture::Unit::NormalMap));
		}

		int hasMetallicMap = mtl->hasMetallicMap();
		if (hasMetallicMap) {
			strongMetallicMap = mtl->m_metallicMap.lock();
			strongMetallicMap->bindToTextureUnit(Texture::Unit::MetallicMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_MetallicMap", int(Texture::Unit::MetallicMap));
		}

		int hasRoughnessMap = mtl->hasRoughnessMap();
		if (hasRoughnessMap) {
			strongroughnessMap = mtl->m_roughnessMap.lock();
			strongroughnessMap->bindToTextureUnit(Texture::Unit::RoughnessMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_RoughnessMap", int(Texture::Unit::RoughnessMap));
		}

		shader->setUniform4("u_HasANRMMap", hasAlbedoMap, hasNormalMap, hasRoughnessMap, hasMetallicMap);
	}

	s_MaterialBlockBuf->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::MaterialBlock));
	s_MaterialBlockBuf->loadSubData(&s_MaterialBlock, 0, sizeof(MaterialBlock));
	shader->bindUniformBlock("MaterialBlock", ShaderProgram::UniformBlockBindingPoint::MaterialBlock);
	shader->bindSubroutineUniforms();

	// draw
	m_renderer->getRenderer()->executeDrawCommand(renderTask.vao, renderTask.primitive, renderTask.vertexCount, renderTask.indexCount);

	s_MaterialBlockBuf->unbind();
	if (strongDiffuseMap) strongDiffuseMap->unbindFromTextureUnit();
	if (strongNormalMap) strongNormalMap->unbindFromTextureUnit();
	if (strongSpecularMap) strongSpecularMap->unbindFromTextureUnit();
	if (strongMetallicMap) strongMetallicMap->unbindFromTextureUnit();
	if (strongroughnessMap) strongroughnessMap->unbindFromTextureUnit();
	if (renderTask.boneCount > 0) s_SkinPoseBlockBuf->unbind();
}


GeometryPassRenderTaskExecutor::GeometryPassRenderTaskExecutor(IRenderTechnique* rt) :RenderTaskExecutor(rt) {

}

void GeometryPassRenderTaskExecutor::executeMeshTask(const MeshRenderItem_t& renderTask, ShaderProgram* shader) {
	auto renderer = static_cast<DeferredRenderer*>(m_renderer);
	std::shared_ptr<Texture> strongDiffuseMap;
	std::shared_ptr<Texture> strongNormalMap;
	std::shared_ptr<Texture> strongSpecularMap;
	std::shared_ptr<Texture> strongMatellicMap;
	std::shared_ptr<Texture> strongRoughnessMap;

	int hasAlbedoMap = 0;
	int hasNormalMap = 0;
	int hasSpecularMap = 0;
	int hasMetallicMap = 0;
	int hasRoughnessMap = 0;

	// set matrixs
	if (shader->hasUniform("u_ModelMat")) {
		shader->setUniformMat4v("u_ModelMat", &renderTask.modelMatrix[0][0]);
	}

	if (shader->hasSubroutineUniform(Shader::Type::VertexShader, "u_Transform")) {
		if (renderTask.boneCount <= 0) {
			shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "staticMesh");
		}
		else {
#ifdef _DEBUG
			ASSERT(renderTask.boneCount <= MAX_NUM_BONES);
#endif // _DEBUG
			int  numBone = MIN(renderTask.boneCount, MAX_NUM_BONES);
			shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "skinMesh");
			s_SkinPoseBlockBuf->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::SkinPoseBlock));
			s_SkinPoseBlockBuf->loadSubData(renderTask.bonesTransform, 0, sizeof(glm::mat4) * numBone);
			shader->bindUniformBlock("SkinPoseBlock", ShaderProgram::UniformBlockBindingPoint::SkinPoseBlock);
		}
	}

	if (auto mtl = renderTask.material->asType<PhongMaterial>()) {
		shader->setSubroutineUniform(Shader::Type::FragmentShader, "u_ShadingMode", "PhongShading");
		
		s_MaterialBlock.albedo = glm::vec4(mtl->m_mainColor, mtl->m_opacity);
		s_MaterialBlock.specular = glm::vec4(mtl->m_specularColor, mtl->m_shininess);
		s_MaterialBlock.emissive = mtl->m_emissive;
		s_MaterialBlock.metalness = 0.f;
		s_MaterialBlock.roughness = 0.f;

		if (mtl->hasAlbedoMap()) {
			strongDiffuseMap = mtl->m_albedoMap.lock();
			hasAlbedoMap = 1;
		}
		if (mtl->hasNormalMap()) {
			strongNormalMap = mtl->m_normalMap.lock();
			hasNormalMap = 1;
		}
		
		if (mtl->hasSpecularMap()) {
			strongSpecularMap = mtl->m_specularMap.lock();
			strongSpecularMap->bindToTextureUnit(Texture::Unit::SpecularMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_SpecularMap", int(Texture::Unit::SpecularMap));
			hasSpecularMap = 1;
		}

		shader->setUniform4("u_HasANRMMap", hasAlbedoMap, hasNormalMap, hasSpecularMap, 0);
	}

	if (auto mtl = renderTask.material->asType<PBRMaterial>()) {
		shader->setSubroutineUniform(Shader::Type::FragmentShader, "u_ShadingMode", "PBRShading");
		
		s_MaterialBlock.albedo = glm::vec4(mtl->m_mainColor, mtl->m_opacity);
		s_MaterialBlock.specular = glm::vec4(0.f);
		s_MaterialBlock.emissive = mtl->m_emissive;
		s_MaterialBlock.metalness = mtl->m_metallic;
		s_MaterialBlock.roughness = mtl->m_roughness;
		
		if (mtl->hasAlbedoMap()) {
			strongDiffuseMap = mtl->m_albedoMap.lock();
			hasAlbedoMap = 1;
		}
		if (mtl->hasNormalMap()) {
			strongNormalMap = mtl->m_normalMap.lock();
			hasNormalMap = 1;
		}
		
		if (mtl->hasMetallicMap()) {
			strongMatellicMap = mtl->m_metallicMap.lock();
			strongMatellicMap->bindToTextureUnit(Texture::Unit::MetallicMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_MetallicMap", int(Texture::Unit::MetallicMap));
			hasMetallicMap = 1;
		}

		if (mtl->hasRoughnessMap()) {
			strongRoughnessMap = mtl->m_roughnessMap.lock();
			strongRoughnessMap->bindToTextureUnit(Texture::Unit::RoughnessMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_RoughnessMap", int(Texture::Unit::RoughnessMap));
			hasRoughnessMap = 1;
		}

		shader->setUniform4("u_HasANRMMap", hasAlbedoMap, hasNormalMap, hasRoughnessMap, hasMetallicMap);
	}


	if (hasAlbedoMap) {
		strongDiffuseMap->bindToTextureUnit(Texture::Unit::DiffuseMap, Texture::Target::Texture_2D);
		shader->setUniform1("u_AlbedoMap", int(Texture::Unit::DiffuseMap));
	}
	if (hasNormalMap) {
		strongNormalMap->bindToTextureUnit(Texture::Unit::NormalMap, Texture::Target::Texture_2D);
		shader->setUniform1("u_NormalMap", int(Texture::Unit::NormalMap));
	}

	s_MaterialBlockBuf->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::MaterialBlock));
	s_MaterialBlockBuf->loadSubData(&s_MaterialBlock, 0, sizeof(MaterialBlock));
	shader->bindUniformBlock("MaterialBlock", ShaderProgram::UniformBlockBindingPoint::MaterialBlock);
	shader->bindSubroutineUniforms();

	// draw
	m_renderer->getRenderer()->executeDrawCommand(renderTask.vao, renderTask.primitive, renderTask.vertexCount, renderTask.indexCount);

	s_MaterialBlockBuf->unbind();
	if (hasAlbedoMap) strongDiffuseMap->unbindFromTextureUnit();
	if (hasNormalMap) strongNormalMap->unbindFromTextureUnit();
	if (hasSpecularMap) strongSpecularMap->unbindFromTextureUnit();
	if (hasMetallicMap) strongMatellicMap->unbindFromTextureUnit();
	if (hasRoughnessMap) strongRoughnessMap->unbindFromTextureUnit();
	if (renderTask.boneCount > 0) s_SkinPoseBlockBuf->unbind();
}


ShadowPassRenderTaskExecutor::ShadowPassRenderTaskExecutor(IRenderTechnique* rt) : RenderTaskExecutor(rt) {

}

void ShadowPassRenderTaskExecutor::executeMeshTask(const MeshRenderItem_t& renderTask, ShaderProgram* shader) {
	if (renderTask.primitive != PrimitiveType::Triangles &&
		renderTask.primitive != PrimitiveType::TriangleFan &&
		renderTask.primitive != PrimitiveType::TriangleStrip)
		return;
	
	// set model matrix
	if (shader->hasUniform("u_ModelMat")) {
		glm::mat4 m = renderTask.modelMatrix;
		shader->setUniformMat4v("u_ModelMat", &m[0][0]);
	}

	if (shader->hasSubroutineUniform(Shader::Type::VertexShader, "u_Transform")) {
		if (renderTask.boneCount <= 0) {
			shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "staticMesh");
		}
		else {
#ifdef _DEBUG
			ASSERT(renderTask.boneCount <= MAX_NUM_BONES);
#endif // _DEBUG
			int  numBone = MIN(renderTask.boneCount, MAX_NUM_BONES);
			shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "skinMesh");
			s_SkinPoseBlockBuf->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::SkinPoseBlock));
			s_SkinPoseBlockBuf->loadSubData(renderTask.bonesTransform, 0, sizeof(glm::mat4) * numBone);
			shader->bindUniformBlock("SkinPoseBlock", ShaderProgram::UniformBlockBindingPoint::SkinPoseBlock);
		}
	}

	shader->bindSubroutineUniforms();
	
	m_renderer->getRenderer()->executeDrawCommand(renderTask.vao, renderTask.primitive, renderTask.vertexCount, renderTask.indexCount);

	if (renderTask.boneCount > 0) s_SkinPoseBlockBuf->unbind();
}



AmbientPassRenderTaskExecutor::AmbientPassRenderTaskExecutor(IRenderTechnique* rt): RenderTaskExecutor(rt) {

}


void AmbientPassRenderTaskExecutor::executeMeshTask(const MeshRenderItem_t& task, ShaderProgram* shader) {
	std::shared_ptr<Texture> diffuseMap;
	std::shared_ptr<Texture> normalMap;

	// set model matrix
	if (shader->hasUniform("u_ModelMat")) {
		shader->setUniformMat4v("u_ModelMat", (float*)&task.modelMatrix[0][0]);
	}

	if (shader->hasSubroutineUniform(Shader::Type::VertexShader, "u_Transform")) {
		if (task.boneCount <= 0) {
			shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "staticMesh");
		}
		else {
#ifdef _DEBUG
			ASSERT(task.boneCount <= MAX_NUM_BONES);
#endif // _DEBUG
			int  numBone = MIN(task.boneCount, MAX_NUM_BONES);
			shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "skinMesh");
			s_SkinPoseBlockBuf->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::SkinPoseBlock));
			s_SkinPoseBlockBuf->loadSubData(task.bonesTransform, 0, sizeof(glm::mat4) * numBone);
			shader->bindUniformBlock("SkinPoseBlock", ShaderProgram::UniformBlockBindingPoint::SkinPoseBlock);
		}
	}

	int hasDiffuseMap = 0;
	int hasNormalMap = 0;
	if (auto mtl = task.material->asType<PhongMaterial>()) {
		hasDiffuseMap = mtl->hasAlbedoMap();
		hasNormalMap = mtl->hasNormalMap();
		if (hasDiffuseMap)
			diffuseMap = mtl->m_albedoMap.lock();
		if (hasNormalMap)
			normalMap = mtl->m_normalMap.lock();

	} else if (auto mtl = task.material->asType<PBRMaterial>()) {
		hasDiffuseMap = mtl->hasAlbedoMap();
		hasNormalMap = mtl->hasNormalMap();
		if (hasDiffuseMap)
			diffuseMap = mtl->m_albedoMap.lock();
		if (hasNormalMap)
			normalMap = mtl->m_normalMap.lock();
	}

	if (hasDiffuseMap) {
		diffuseMap->bindToTextureUnit(Texture::Unit::DiffuseMap);
		shader->setUniform1("u_AlbedoMap", int(Texture::Unit::DiffuseMap));
	}

	if (hasNormalMap) {
		normalMap->bindToTextureUnit(Texture::Unit::NormalMap);
		shader->setUniform1("u_NormalMap", int(Texture::Unit::NormalMap));
	}

	shader->setUniform2("u_HasANMap", hasDiffuseMap, hasNormalMap);
	shader->bindSubroutineUniforms();

	m_renderer->getRenderer()->executeDrawCommand(task.vao, task.primitive, task.vertexCount, task.indexCount);
	
	if (diffuseMap) diffuseMap->unbindFromTextureUnit();
	if (normalMap) normalMap->unbindFromTextureUnit();
	if (task.boneCount > 0) s_SkinPoseBlockBuf->unbind();
}



LightAccumulationPassRenderTaskExecutor::LightAccumulationPassRenderTaskExecutor(IRenderTechnique* rt) 
	: RenderTaskExecutor(rt) {

}


void LightAccumulationPassRenderTaskExecutor::executeMeshTask(const MeshRenderItem_t& task, ShaderProgram* shader) {
	std::shared_ptr<Texture> strongDiffuseMap;
	std::shared_ptr<Texture> strongNormalMap;
	std::shared_ptr<Texture> strongSpecularMap;
	std::shared_ptr<Texture> strongMetallicMap;
	std::shared_ptr<Texture> strongroughnessMap;

	if (shader->hasUniform("u_ModelMat")) {
		shader->setUniformMat4v("u_ModelMat", &task.modelMatrix[0][0]);
	}

	if (shader->hasSubroutineUniform(Shader::Type::VertexShader, "u_Transform")) {
		if (task.boneCount <= 0) {
			shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "staticMesh");
		}
		else {
#ifdef _DEBUG
			ASSERT(task.boneCount <= MAX_NUM_BONES);
#endif // _DEBUG
			int  numBone = MIN(task.boneCount, MAX_NUM_BONES);
			shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "skinMesh");
			s_SkinPoseBlockBuf->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::SkinPoseBlock));
			s_SkinPoseBlockBuf->loadSubData(task.bonesTransform, 0, sizeof(glm::mat4) * numBone);
			shader->bindUniformBlock("SkinPoseBlock", ShaderProgram::UniformBlockBindingPoint::SkinPoseBlock);
		}
	}

	if (auto mtl = task.material->asType<PhongMaterial>()) {
		shader->setSubroutineUniform(Shader::Type::FragmentShader, "u_ShadingMode", "PhongShading");

		s_MaterialBlock.albedo = glm::vec4(mtl->m_mainColor, mtl->m_opacity);
		s_MaterialBlock.specular = glm::vec4(mtl->m_specularColor, mtl->m_shininess * PhongMaterial::s_maxShininess);
		s_MaterialBlock.emissive = mtl->m_emissive;
		s_MaterialBlock.metalness = 0.f;
		s_MaterialBlock.roughness = 0.f;

		// set textures
		int hasAlbedoMap = mtl->hasAlbedoMap();
		if (hasAlbedoMap) {
			strongDiffuseMap = mtl->m_albedoMap.lock();
			strongDiffuseMap->bindToTextureUnit(Texture::Unit::DiffuseMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_AlbedoMap", int(Texture::Unit::DiffuseMap));
		}

		int hasNormalMap = mtl->hasNormalMap();
		if (hasNormalMap) {
			strongNormalMap = mtl->m_normalMap.lock();
			strongNormalMap->bindToTextureUnit(Texture::Unit::NormalMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_NormalMap", int(Texture::Unit::NormalMap));
		}

		int hasSpecMap = mtl->hasSpecularMap();
		if (hasSpecMap) {
			strongSpecularMap = mtl->m_specularMap.lock();
			strongSpecularMap->bindToTextureUnit(Texture::Unit::SpecularMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_SpecularMap", int(Texture::Unit::SpecularMap));
		}

		shader->setUniform4("u_HasANRMMap", hasAlbedoMap, hasNormalMap, hasSpecMap, 0);
	}

	if (auto mtl = task.material->asType<PBRMaterial>()) {
		shader->setSubroutineUniform(Shader::Type::FragmentShader, "u_ShadingMode", "PBRShading");

		s_MaterialBlock.albedo = glm::vec4(mtl->m_mainColor, mtl->m_opacity);
		s_MaterialBlock.specular = glm::vec4(0.f);
		s_MaterialBlock.emissive = mtl->m_emissive;
		s_MaterialBlock.metalness = mtl->m_metallic;
		s_MaterialBlock.roughness = mtl->m_roughness;

		// set textures
		int hasAlbedoMap = mtl->hasAlbedoMap();
		if (hasAlbedoMap) {
			strongDiffuseMap = mtl->m_albedoMap.lock();
			strongDiffuseMap->bindToTextureUnit(Texture::Unit::DiffuseMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_AlbedoMap", int(Texture::Unit::DiffuseMap));
		}

		int hasNormalMap = mtl->hasNormalMap();
		if (hasNormalMap) {
			strongNormalMap = mtl->m_normalMap.lock();
			strongNormalMap->bindToTextureUnit(Texture::Unit::NormalMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_NormalMap", int(Texture::Unit::NormalMap));
		}

		int hasMetallicMap = mtl->hasMetallicMap();
		if (hasMetallicMap) {
			strongMetallicMap = mtl->m_metallicMap.lock();
			strongMetallicMap->bindToTextureUnit(Texture::Unit::MetallicMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_MetallicMap", int(Texture::Unit::MetallicMap));
		}

		int hasRoughnessMap = mtl->hasRoughnessMap();
		if (hasRoughnessMap) {
			strongroughnessMap = mtl->m_roughnessMap.lock();
			strongroughnessMap->bindToTextureUnit(Texture::Unit::RoughnessMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_RoughnessMap", int(Texture::Unit::RoughnessMap));
		}

		shader->setUniform4("u_HasANRMMap", hasAlbedoMap, hasNormalMap, hasRoughnessMap, hasMetallicMap);
	}

	s_MaterialBlockBuf->bindBase(Buffer::Target::UniformBuffer, int(ShaderProgram::UniformBlockBindingPoint::MaterialBlock));
	s_MaterialBlockBuf->loadSubData(&s_MaterialBlock, 0, sizeof(MaterialBlock));
	shader->bindUniformBlock("MaterialBlock", ShaderProgram::UniformBlockBindingPoint::MaterialBlock);
	shader->bindSubroutineUniforms();

	// draw
	m_renderer->getRenderer()->executeDrawCommand(task.vao, task.primitive, task.vertexCount, task.indexCount);
	
	s_MaterialBlockBuf->unbind();
	if (strongDiffuseMap) strongDiffuseMap->unbindFromTextureUnit();
	if (strongNormalMap) strongNormalMap->unbindFromTextureUnit();
	if (strongSpecularMap) strongSpecularMap->unbindFromTextureUnit();
	if (strongMetallicMap) strongMetallicMap->unbindFromTextureUnit();
	if (strongroughnessMap) strongroughnessMap->unbindFromTextureUnit();
	if (task.boneCount > 0) s_SkinPoseBlockBuf->unbind();
}