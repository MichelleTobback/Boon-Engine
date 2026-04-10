#include "OpenGLApi.h"
#include "Renderer/VertexInput.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "BoonDebug/Logger.h"

using namespace Boon;

void OpenGLMessageCallback(
    unsigned source,
    unsigned type,
    unsigned id,
    unsigned severity,
    int,
    const char* message,
    const void*)
{
    std::string sourceStr;
    switch (source) 
    {
    case GL_DEBUG_SOURCE_API:
        sourceStr = "API";
        break;

    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        sourceStr = "WINDOW SYSTEM";
        break;

    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        sourceStr = "SHADER COMPILER";
        break;

    case GL_DEBUG_SOURCE_THIRD_PARTY:
        sourceStr = "THIRD PARTY";
        break;

    case GL_DEBUG_SOURCE_APPLICATION:
        sourceStr = "APPLICATION";
        break;

    case GL_DEBUG_SOURCE_OTHER:
        sourceStr = "UNKNOWN";
        break;

    default:
        sourceStr = "UNKNOWN";
        break;
    }

    std::string typeStr;
    switch (type) 
    {
    case GL_DEBUG_TYPE_ERROR:
        typeStr = "ERROR";
        break;

    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        typeStr = "DEPRECATED BEHAVIOR";
        break;

    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        typeStr = "UDEFINED BEHAVIOR";
        break;

    case GL_DEBUG_TYPE_PORTABILITY:
        typeStr = "PORTABILITY";
        break;

    case GL_DEBUG_TYPE_PERFORMANCE:
        typeStr = "PERFORMANCE";
        break;

    case GL_DEBUG_TYPE_OTHER:
        typeStr = "OTHER";
        break;

    case GL_DEBUG_TYPE_MARKER:
        typeStr = "MARKER";
        break;

    default:
        typeStr = "UNKNOWN";
        break;
    }

    switch (severity) 
    {
    case GL_DEBUG_SEVERITY_HIGH:
        BOON_LOG_ERROR(message);
        break;

    case GL_DEBUG_SEVERITY_MEDIUM:
    case GL_DEBUG_SEVERITY_LOW:
        BOON_LOG_WARN(message);
        break;

    default:
        BOON_LOG("[OpenGL][{}] ({}) ID {} : {}", sourceStr, typeStr, id, message);
        break;
    }
}

void OpenGLApi::Init()
{
    // Load OpenGL function pointers using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 
    {
        BOON_LOG_ERROR("Failed to initialize GLAD");
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
