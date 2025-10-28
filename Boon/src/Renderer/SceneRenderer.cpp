#include "Renderer/SceneRenderer.h"
#include "Renderer/Renderer.h"

#include "Renderer/VertexInput.h"
#include "Renderer/Shader.h"

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

	std::string vertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = vec4(a_Position, 1.0);	
			}
		)";

	std::string fragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;

			void main()
			{
				color = v_Color;
			}
		)";

	m_pShader = Shader::Create(vertexSrc, fragmentSrc);
}

void Boon::SceneRenderer::Render()
{
	Renderer::BeginFrame();

	m_pShader->Bind();
	m_pQuadVertexInput->Bind();

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	Renderer::EndFrame();
}
