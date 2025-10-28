#include "Renderer/VertexInput.h"

#include "Platform/OpenGL/OpenGLVertexInput.h"

using namespace Boon;

std::shared_ptr<VertexInput> Boon::VertexInput::Create()
{
	return std::make_shared<OpenGLVertexInput>();
}
