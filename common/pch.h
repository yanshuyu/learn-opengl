#pragma once
#include<glad/glad.h>
#define GLFW_INCLUDE_NONE

// application
#include"Containers.h"
#include"Util.h"
#include"Exceptions.h"
#include"NotificationCenter.h"
#include"ShaderProgamMgr.h"
#include"TextureMgr.h"
#include"MeshMgr.h"
#include"MaterialMgr.h"
#include"InputMgr.h"
#include"GuiMgr.h"
#include"MeshLoader.h"
#include"FileSystem.h"

// opengl wrapper
#include<GLFW/glfw3.h>
#include"Buffer.h"
#include"FrameBuffer.h"
#include"RenderBuffer.h"
#include"VertexLayoutDescription.h"
#include"VertexArray.h"
#include"RenderBuffer.h"

// renderring & animation
#include"Transform.h"
#include"PhongMaterial.h"
#include"PBRMaterial.h"
#include"Model.h"
#include"Scene.h"
#include"SceneObject.h"
#include"Interpolation.h"
#include"Skeleton.h"
#include"AnimationClip.h"
#include"Renderer.h"
#include"DebugDrawer.h"

// components
#include"TransformComponent.h"
#include"MeshRenderComponent.h"
#include"SkinMeshRenderComponent.h"
#include"CameraComponent.h"
#include"LightComponent.h"
#include"AnimatorComponent.h"
#include"FirstPersonCameraController.h"
#include"ArcballCameraController.h"
#include"SkyboxComponent.h"
#include"HDRFilter.h"
#include"HDRFilter2.h"
#include"GrayFilter.h"
#include"GaussianBlurFilter.h"