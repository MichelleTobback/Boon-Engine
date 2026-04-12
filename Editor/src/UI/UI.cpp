#include "UI/UI.h"

namespace BoonEditor
{
	std::unordered_map<std::string, UI::PendingPropertyState> UI::m_PendingPropertyEdits{};
}