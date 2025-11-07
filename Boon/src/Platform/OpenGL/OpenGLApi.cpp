#include "OpenGLApi.h"
#include "Renderer/VertexInput.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

using namespace Boon;

void OpenGLApi::Init()
{
    // Load OpenGL function pointers using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return;
    }
}

void OpenGLApi::Shutdown()
{

}

void OpenGLApi::BeginFrame()
{
    
}

void OpenGLApi::EndFrame()
{
    
}

void Boon::OpenGLApi::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    glViewport(x, y, width, height);
}

void Boon::OpenGLApi::SetClearColor(const glm::vec4& color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

void Boon::OpenGLApi::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Boon::OpenGLApi::DrawIndexed(const std::shared_ptr<VertexInput>& vertexInput, uint32_t indexCount)
{
    vertexInput->Bind();
    uint32_t count = indexCount ? indexCount : vertexInput->GetIndexBuffer()->GetCount();
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
}

void Boon::OpenGLApi::DrawLines(const std::shared_ptr<VertexInput>& vertexInput, uint32_t lineCount)
{
    vertexInput->Bind();
    uint32_t count = lineCount ? lineCount * 2 : vertexInput->GetIndexBuffer()->GetCount();
    glLineWidth(2.f);
    glDrawArrays(GL_LINES, 0, count);
}
