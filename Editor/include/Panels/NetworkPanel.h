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

		void SetDriver(NetDriver* pDriver);

	protected:
		virtual void OnRenderUI() override;

	private:
		NetDriver* m_pDriver{nullptr};
	};
}