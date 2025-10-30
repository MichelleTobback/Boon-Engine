#include "Renderer/SceneRenderer.h"
#include "Renderer/Renderer.h"

#include "Renderer/VertexInput.h"
#include "Renderer/Shader.h"
#include "Renderer/UniformBuffer.h"
#include "Renderer/UBData.h"
#include "Renderer/Camera.h"

#include "Core/ServiceLocator.h"

#include "Asset/AssetLibrary.h"
#include "Asset/ShaderAsset.h"

//temp
#include "Input/Input.h"
#include "Core/Time.h"
#include "Core/Application.h"
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>

using namespace Boon;

Boon::SceneRenderer::SceneRenderer()
{
	float quadVerts[(3 + 4) * 4]
	{
		-0.5f, -0.5f, 0.f, 1.f, 0.f, 0.f, 1.f,
		 0.5f, -0.5f, 0.f, 0.f, 1.f, 0.f, 1.f,
		 0.5f,  0.5f, 0.f, 0.f, 0.f, 1.f, 1.f,
		-0.5f,  0.5f, 0.f, 1.f, 1.f, 0.f, 1.f
	};

	uint32_t quadIndices[6]{ 0, 1, 2, 2, 3, 0 };

	m_pQuadVertexInput = VertexInput::Create();

	std::shared_ptr<VertexBuffer> pQuadVertexBuffer{ VertexBuffer::Create(quadVerts, sizeof(quadVerts)) };
	pQuadVertexBuffer->SetLayout({
		{ShaderDataType::Float3, "a_Position"},
		{ShaderDataType::Float4, "a_Color"}
	});
	m_pQuadVertexInput->AddVertexBuffer(pQuadVertexBuffer);
	m_pQuadVertexInput->SetIndexBuffer(IndexBuffer::Create(quadIndices, sizeof(quadIndices) / sizeof(uint32_t)));

	AssetLibrary& assets{ ServiceLocator::Get<AssetLibrary>()};
	AssetHandle quadHandle = assets.Load<ShaderAssetLoader>("shaders/Quad.glsl");
	m_pShader = assets.GetAsset<ShaderAsset>(quadHandle);

	m_pCameraUniformBuffer = UniformBuffer::Create<UBData::Camera>(0);
	m_pCamera = std::make_shared<Camera>( 90.f, (float)Application::Get().GetWindow().GetWidth(), (float)Application::Get().GetWindow().GetHeight(), 0.01f, 1000.f );
	m_CameraData.ViewProjection = m_pCamera->GetProjection() * (glm::translate(glm::mat4(1.f), { 0.f, 0.f, -1.f }));
}

void Boon::SceneRenderer::Render()
{
	Renderer::BeginFrame();

	Input& input = ServiceLocator::Get<Input>();
	Window& window = Application::Get().GetWindow();
	glm::vec3 pos{0.f, 0.f, -1.f};

	m_CameraData.ViewProjection = m_pCamera->GetProjection() * (glm::translate(glm::mat4(1.f), pos));
	m_pCameraUniformBuffer->SetValue(m_CameraData);

	m_pShader->Bind();
	m_pQuadVertexInput->Bind();

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	Renderer::EndFrame();
}
