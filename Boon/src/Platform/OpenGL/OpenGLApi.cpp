#include "OpenGLApi.h"

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
    glClearColor(0.9f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLApi::EndFrame()
{
    
}