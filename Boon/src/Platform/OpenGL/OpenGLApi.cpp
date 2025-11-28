#include "OpenGLApi.h"
#include "Renderer/VertexInput.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

using namespace Boon;

void OpenGLMessageCallback(
    unsigned source,
    unsigned type,
    unsigned id,
    unsigned severity,
    int length,
    const char* message,
    const void* userParam)
{
    std::cout << message << '\n';
}

void OpenGLApi::Init()
{
    // Load OpenGL function pointers using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return;
    }

    //debug
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenGLMessageCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
    //debug

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
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
    uint32_t count = indexCount ? indexCount : vertexInput->GetIndexBuffer()->GetCount();

    if (count > 0)
    {
        vertexInput->Bind();
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
    }
}

void Boon::OpenGLApi::DrawArrays(const std::shared_ptr<VertexInput>& vertexInput, uint32_t indexCount)
{
    vertexInput->Bind();
    uint32_t count = indexCount ? indexCount : vertexInput->GetIndexBuffer() ? vertexInput->GetIndexBuffer()->GetCount() : 0;
    glDrawArrays(GL_TRIANGLES, 0, count);
}

void Boon::OpenGLApi::DrawLines(const std::shared_ptr<VertexInput>& vertexInput, uint32_t lineCount)
{
    vertexInput->Bind();
    uint32_t count = lineCount ? lineCount * 2 : vertexInput->GetIndexBuffer()->GetCount();
    glLineWidth(2.f);
    glDrawArrays(GL_LINES, 0, count);
}
