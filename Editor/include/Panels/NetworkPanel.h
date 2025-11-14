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
		NetworkPanel(const std::string& name, Boon::NetworkSettings& settings);

		void SetDriver(NetDriver* pDriver);

	protected:
		virtual void OnRenderUI() override;

	private:
		Boon::NetworkSettings& m_Settings;
		NetDriver* m_pDriver{nullptr};
	};
}