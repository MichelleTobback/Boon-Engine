#include "Core/EditorContext.h"

#include "Command/EditorCommandQueue.h"

using namespace Boon;

namespace BoonEditor
{
	EditorContext::EditorContext()
		: m_CommandQueue{ std::make_unique<EditorCommandQueue>() }{}
}