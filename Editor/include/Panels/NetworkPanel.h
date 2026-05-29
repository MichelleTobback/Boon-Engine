#pragma once
#include "EditorPanel.h"

#include <Networking/NetworkSettings.h>

namespace Boon
{
	class NetDriver;
}

using namespace Boon;

namespace BoonEditor
{
	class NetworkPanel final : public EditorPanel
	{
	public:
		NetworkPanel(EditorContext* pContext, const std::string& name);

	protected:
		virtual void OnRenderUI() override;

	private:
#ifdef BOON_WITH_NETWORKING
		NetDriver* m_pDriver{nullptr};
#endif
	};
}