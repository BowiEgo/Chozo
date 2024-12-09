#pragma once

// For use by Chozo applications, which means they can be accessed by client.
#include "Chozo/Core/Application.h"
#include "Chozo/Core/Layer.h"
#include "Chozo/Core/Timestep.h"
#include "Chozo/Core/Ref.h"
#include "Chozo/Core/Input.h"
#include "Chozo/Core/KeyCodes.h"
#include "Chozo/Core/MouseButtonCodes.h"
#include "Chozo/Core/Pool.h"

#include "Chozo/Debug/Log.h"

#include "Chozo/ImGui/ImGuiLayer.h"

#include "Chozo/Scene/Scene.h"
#include "Chozo/Scene/Entity.h"
#include "Chozo/Scene/Components.h"

// ---Renderer---------------------
#include "Chozo/Renderer/Renderer.h"
#include "Chozo/Renderer/Renderer2D.h"
#include "Chozo/Renderer/RenderCommand.h"
#include "Chozo/Renderer/SceneRenderer.h"

#include "Chozo/Renderer/Pipeline.h"
#include "Chozo/Renderer/Shader.h"
#include "Chozo/Renderer/Texture.h"
#include "Chozo/Renderer/VertexBuffer.h"
#include "Chozo/Renderer/IndexBuffer.h"
#include "Chozo/Renderer/VertexArray.h"
#include "Chozo/Renderer/Framebuffer.h"

#include "Chozo/Renderer/OrthographicCamera.h"
// --------------------------------

// ---Entry Point------------------
#ifdef CHOZO_ENTRY_POINT
#include "Chozo/EntryPoint.h"
#endif
// --------------------------------