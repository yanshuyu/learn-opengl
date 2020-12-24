#include"RenderTaskExecutor.h"
#include"VertexArray.h"
#include"PhongMaterial.h"
#include"PBRMaterial.h"
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
			shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "staticMesh");
		}
		else {
#ifdef _DEBUG
			ASSERT(renderTask.boneCount <= MAX_NUM_BONE);
#endif // _DEBUG
			int  numBone = MIN(renderTask.boneCount, MAX_NUM_BONE);
			shader->setUniformMat4v("u_SkinPose[0]", (float*)glm::value_ptr(renderTask.bonesTransform[0]), numBone);
			shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "skinMesh");
		}
	}

	shader->bindSubroutineUniforms();

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
				shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "staticMesh");
			}
			else {
#ifdef _DEBUG
				ASSERT(renderTask.boneCount <= MAX_NUM_BONE);
#endif // _DEBUG
				int  numBone = MIN(renderTask.boneCount, MAX_NUM_BONE);
				shader->setUniformMat4v("u_SkinPose[0]", (float*)glm::value_ptr(renderTask.bonesTransform[0]), numBone);
				shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "skinMesh");
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
	if (strongDiffuseMap)
		strongDiffuseMap->unbind();
}



LightPassRenderTaskExecuter::LightPassRenderTaskExecuter(IRenderTechnique* rt) : RenderTaskExecutor(rt) {

}


void LightPassRenderTaskExecuter::executeMeshTask(const MeshRenderItem_t& renderTask, ShaderProgram* shader) {
	renderTask.vao->bind();
	std::shared_ptr<Texture> strongDiffuseMap;
	std::shared_ptr<Texture> strongNormalMap;
	std::shared_ptr<Texture> strongSpecularMap;
	std::shared_ptr<Texture> strongMetallicMap;
	std::shared_ptr<Texture> strongroughnessMap;

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
			ASSERT(renderTask.boneCount <= MAX_NUM_BONE);
#endif // _DEBUG
			int  numBone = MIN(renderTask.boneCount, MAX_NUM_BONE);
			shader->setUniformMat4v("u_SkinPose[0]", (float*)glm::value_ptr(renderTask.bonesTransform[0]), numBone);
			shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "skinMesh");
		}
	}
	
	if (auto mtl = renderTask.material->asType<PhongMaterial>()) {
		shader->setSubroutineUniform(Shader::Type::FragmentShader, "u_ShadingMode", "PhongShading");

		shader->setUniform4("u_mainColor", mtl->m_mainColor.r, mtl->m_mainColor.g, mtl->m_mainColor.b, mtl->m_opacity);
		shader->setUniform4("u_specularColor", mtl->m_specularColor.r, mtl->m_specularColor.g, mtl->m_specularColor.b, mtl->m_shininess);
		shader->setUniform1("u_maxShininess", PhongMaterial::s_maxShininess);

		// set textures
		int hasAlbedoMap = mtl->hasAlbedoMap();
		if (hasAlbedoMap) {
			strongDiffuseMap = mtl->m_albedoMap.lock();
			strongDiffuseMap->bind(Texture::Unit::DiffuseMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_albedoMap", int(Texture::Unit::DiffuseMap));
		}
	
		int hasNormalMap = mtl->hasNormalMap();
		if (hasNormalMap) {
			strongNormalMap = mtl->m_normalMap.lock();
			strongNormalMap->bind(Texture::Unit::NormalMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_normalMap", int(Texture::Unit::NormalMap));
		}	

		int hasSpecMap = mtl->hasSpecularMap();
		if (hasSpecMap) {
			strongSpecularMap = mtl->m_specularMap.lock();
			strongSpecularMap->bind(Texture::Unit::SpecularMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_specularMap", int(Texture::Unit::SpecularMap));
		}

		shader->setUniform4("u_hasANRMMap", hasAlbedoMap, hasNormalMap, hasSpecMap, 0);
	}

	if (auto mtl = renderTask.material->asType<PBRMaterial>()) {
		shader->setSubroutineUniform(Shader::Type::FragmentShader, "u_ShadingMode", "PBRShading");

		shader->setUniform4("u_mainColor", mtl->m_mainColor.r, mtl->m_mainColor.g, mtl->m_mainColor.b, mtl->m_opacity);
		shader->setUniform1("u_roughness", mtl->m_roughness);
		shader->setUniform1("u_metalness", mtl->m_metallic);

		// set textures
		int hasAlbedoMap = mtl->hasAlbedoMap();
		if (hasAlbedoMap) {
			strongDiffuseMap = mtl->m_albedoMap.lock();
			strongDiffuseMap->bind(Texture::Unit::DiffuseMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_albedoMap", int(Texture::Unit::DiffuseMap));
		}

		int hasNormalMap = mtl->hasNormalMap();
		if (hasNormalMap) {
			strongNormalMap = mtl->m_normalMap.lock();
			strongNormalMap->bind(Texture::Unit::NormalMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_normalMap", int(Texture::Unit::NormalMap));
		}

		int hasMetallicMap = mtl->hasMetallicMap();
		if (hasMetallicMap) {
			strongMetallicMap = mtl->m_metallicMap.lock();
			strongMetallicMap->bind(Texture::Unit::MetallicMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_metallicMap", int(Texture::Unit::MetallicMap));
		}

		int hasRoughnessMap = mtl->hasRoughnessMap();
		if (hasRoughnessMap) {
			strongroughnessMap = mtl->m_roughnessMap.lock();
			strongroughnessMap->bind(Texture::Unit::RoughnessMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_roughnessMap", int(Texture::Unit::RoughnessMap));
		}

		shader->setUniform4("u_hasANRMMap", hasAlbedoMap, hasNormalMap, hasRoughnessMap, hasMetallicMap);
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
	if (strongDiffuseMap)
		strongDiffuseMap->unbind();
	if (strongNormalMap)
		strongNormalMap->unbind();
	if (strongSpecularMap)
		strongSpecularMap->unbind();
	if (strongMetallicMap)
		strongMetallicMap->unbind();
	if (strongroughnessMap)
		strongroughnessMap->unbind();
}


GeometryPassRenderTaskExecutor::GeometryPassRenderTaskExecutor(IRenderTechnique* rt) :RenderTaskExecutor(rt) {

}

void GeometryPassRenderTaskExecutor::executeMeshTask(const MeshRenderItem_t& renderTask, ShaderProgram* shader) {
	auto renderer = static_cast<DeferredRenderer*>(m_renderer);
	std::shared_ptr<Texture> strongDiffuseMap;
	std::shared_ptr<Texture> strongNormalMap;
	std::shared_ptr<Texture> strongSpecularMap;
	std::shared_ptr<Texture> strongEmissiveMap;
	std::shared_ptr<Texture> strongMatellicMap;
	std::shared_ptr<Texture> strongRoughnessMap;

	renderTask.vao->bind();

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
			ASSERT(renderTask.boneCount <= MAX_NUM_BONE);
#endif // _DEBUG
			int  numBone = MIN(renderTask.boneCount, MAX_NUM_BONE);
			shader->setUniformMat4v("u_SkinPose[0]", (float*)glm::value_ptr(renderTask.bonesTransform[0]), numBone);
			shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "skinMesh");
		}
	}

	if (auto mtl = renderTask.material->asType<PhongMaterial>()) {
		shader->setSubroutineUniform(Shader::Type::FragmentShader, "u_ShadingMode", "PhongShading");

		shader->setUniform4("u_mainColor", mtl->m_mainColor.r, mtl->m_mainColor.g, mtl->m_mainColor.b, mtl->m_opacity);
		shader->setUniform3("u_emissiveColor", mtl->m_emissiveColor.r, mtl->m_emissiveColor.g, mtl->m_emissiveColor.b);
		shader->setUniform4("u_specularColor", mtl->m_specularColor.r, mtl->m_specularColor.g, mtl->m_specularColor.b, mtl->m_shininess);

		if (mtl->hasAlbedoMap()) strongDiffuseMap = mtl->m_albedoMap.lock();
		if (mtl->hasNormalMap()) strongNormalMap = mtl->m_normalMap.lock();
		if (mtl->hasEmissiveMap()) strongEmissiveMap = mtl->m_emissiveMap.lock();
		
		if (mtl->hasSpecularMap()) {
			strongSpecularMap = mtl->m_specularMap.lock();
			strongSpecularMap->bind(Texture::Unit::SpecularMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_specularMap", int(Texture::Unit::SpecularMap));
		}
	}

	if (auto mtl = renderTask.material->asType<PBRMaterial>()) {
		shader->setSubroutineUniform(Shader::Type::FragmentShader, "u_ShadingMode", "PBRShading");

		shader->setUniform4("u_mainColor", mtl->m_mainColor.r, mtl->m_mainColor.g, mtl->m_mainColor.b, mtl->m_opacity);
		shader->setUniform3("u_emissiveColor", mtl->m_emissiveColor.r, mtl->m_emissiveColor.g, mtl->m_emissiveColor.b);
		
		if (mtl->hasAlbedoMap()) strongDiffuseMap = mtl->m_albedoMap.lock();
		if (mtl->hasNormalMap()) strongNormalMap = mtl->m_normalMap.lock();
		if (mtl->hasEmissiveMap()) strongEmissiveMap = mtl->m_emissiveMap.lock();
		
		if (mtl->hasMetallicMap()) {
			strongMatellicMap = mtl->m_metallicMap.lock();
			strongMatellicMap->bind(Texture::Unit::MetallicMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_metallicMap", int(Texture::Unit::MetallicMap));
			shader->setUniform1("u_metalness", mtl->m_metallic);
		}

		if (mtl->hasRoughnessMap()) {
			strongRoughnessMap = mtl->m_roughnessMap.lock();
			strongRoughnessMap->bind(Texture::Unit::RoughnessMap, Texture::Target::Texture_2D);
			shader->setUniform1("u_roughnessMap", int(Texture::Unit::RoughnessMap));
			shader->setUniform1("u_roughness", mtl->m_roughness);
		}
	}

	int hasAlbedoMap = strongDiffuseMap != nullptr;
	int hasNormalMap = strongNormalMap != nullptr;
	int hasEmissiveMap = strongEmissiveMap != nullptr;
	int hasSpecularMap = strongSpecularMap != nullptr;
	int hasMetallicMap = strongMatellicMap != nullptr;
	int hasRoughnessMap = strongRoughnessMap != nullptr;

	if (hasAlbedoMap) {
		strongDiffuseMap->bind(Texture::Unit::DiffuseMap, Texture::Target::Texture_2D);
		shader->setUniform1("u_albedoMap", int(Texture::Unit::DiffuseMap));
	}
	if (hasNormalMap) {
		strongNormalMap->bind(Texture::Unit::NormalMap, Texture::Target::Texture_2D);
		shader->setUniform1("u_normalMap", int(Texture::Unit::NormalMap));
	}
	if (hasEmissiveMap) {
		strongEmissiveMap->bind(Texture::Unit::EmissiveMap, Texture::Target::Texture_2D);
		shader->setUniform1("u_emissiveMap", int(Texture::Unit::EmissiveMap));
	}

	shader->setUniform3("u_hasANEMap", hasAlbedoMap, hasNormalMap, hasEmissiveMap);
	shader->setUniform3("u_hasSMRMap", hasSpecularMap, hasMetallicMap, hasRoughnessMap);
	
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

	if (hasAlbedoMap)
		strongDiffuseMap->unbind();
	if (hasNormalMap)
		strongNormalMap->unbind();
	if (hasSpecularMap)
		strongSpecularMap->unbind();
	if (hasEmissiveMap)
		strongEmissiveMap->unbind();
	if (hasMetallicMap)
		strongMatellicMap->unbind();
	if (hasRoughnessMap)
		strongRoughnessMap->unbind();

	renderTask.vao->unbind();
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
			shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "staticMesh");
		}
		else {
#ifdef _DEBUG
			ASSERT(renderTask.boneCount <= MAX_NUM_BONE);
#endif // _DEBUG
			int  numBone = MIN(renderTask.boneCount, MAX_NUM_BONE);
			shader->setUniformMat4v("u_SkinPose[0]", (float*)glm::value_ptr(renderTask.bonesTransform[0]), numBone);
			shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "skinMesh");
		}
	}

	shader->bindSubroutineUniforms();

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
			shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "staticMesh");
		}
		else {
#ifdef _DEBUG
			ASSERT(task.boneCount <= MAX_NUM_BONE);
#endif // _DEBUG
			int  numBone = MIN(task.boneCount, MAX_NUM_BONE);
			shader->setUniformMat4v("u_SkinPose[0]", (float*)glm::value_ptr(task.bonesTransform[0]), numBone);
			shader->setSubroutineUniform(Shader::Type::VertexShader, "u_Transform", "skinMesh");
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
		diffuseMap->bind(Texture::Unit::DiffuseMap);
		shader->setUniform1("u_AlbedoMap", int(Texture::Unit::DiffuseMap));
	}

	if (hasNormalMap) {
		normalMap->bind(Texture::Unit::NormalMap);
		shader->setUniform1("u_NormalMap", int(Texture::Unit::NormalMap));
	}

	shader->setUniform2("u_HasANMap", hasDiffuseMap, hasNormalMap);
	shader->bindSubroutineUniforms();

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