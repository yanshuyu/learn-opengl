#include"RenderTechnique.h"
#include"Renderer.h"
#include"Util.h"
#include<glad/glad.h>


RenderTechnique::RenderTechnique(Renderer* invoker): m_invoker(invoker)
, m_activeShader(nullptr) {

}
