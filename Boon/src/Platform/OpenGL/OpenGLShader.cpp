#include "OpenGLShader.h"
#include "BoonDebug/Logger.h"

#include <glad/glad.h>
#include <vector>
#include <string>

using namespace Boon;

static void LogShaderError(const char* stage, const std::vector<GLchar>& infoLog)
{
    BOON_LOG_ERROR("{} shader error:\n{}", stage, infoLog.data());
}

Boon::OpenGLShader::OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc)
{
    m_ID = 0;

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

    const GLchar* source = vertexSrc.c_str();
    glShaderSource(vertexShader, 1, &source, nullptr);
    glCompileShader(vertexShader);

    GLint isCompiled = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);

    if (isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(vertexShader, maxLength, &maxLength, infoLog.data());

        BOON_LOG_ERROR("Vertex shader compile failed:\n{}", infoLog.data());

        glDeleteShader(vertexShader);
        return;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    source = fragmentSrc.c_str();
    glShaderSource(fragmentShader, 1, &source, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);

    if (isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, infoLog.data());

        BOON_LOG_ERROR("Fragment shader compile failed:\n{}", infoLog.data());

        glDeleteShader(fragmentShader);
        glDeleteShader(vertexShader);
        return;
    }

    GLuint program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);

    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());

        BOON_LOG_ERROR("Shader link failed:\n{}", infoLog.data());

        glDeleteProgram(program);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        m_ID = 0;
        return;
    }

    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    m_ID = program;
}

Boon::OpenGLShader::~OpenGLShader()
{
    if (m_ID != 0)
        glDeleteProgram(m_ID);
}

void Boon::OpenGLShader::Bind() const
{
    if (m_ID == 0)
    {
        BOON_LOG_ERROR("Trying to bind invalid OpenGL shader program.");
        return;
    }

    glUseProgram(m_ID);
}

void Boon::OpenGLShader::Unbind() const
{
    glUseProgram(0);
}
