#include"RenderTechnique.h"


RenderTechniqueBase::RenderTechniqueBase(Renderer* renderer) :IRenderTechnique(renderer)
, m_pass(RenderPass::None)
, m_passShader() {

}


void RenderTechniqueBase::render(const Scene_t& scene) {
	beginFrame();

	drawDepthPass(scene);
	drawGeometryPass(scene);
	drawOpaquePass(scene);
	drawTransparentPass(scene);

	endFrame();
}
