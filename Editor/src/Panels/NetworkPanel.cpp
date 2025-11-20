#include "Panels/NetworkPanel.h"
#include "UI/UI.h"

#include <sstream>
#include <Networking/NetDriver.h>

using namespace BoonEditor;

using namespace Boon;

NetworkPanel::NetworkPanel(const std::string& name, DragDropRouter* pRouter, Boon::NetworkSettings& settings)
	: EditorPanel(name, pRouter), m_Settings(settings){}

void NetworkPanel::SetDriver(NetDriver* pDriver)
{
	m_pDriver = pDriver;
}

void BoonEditor::NetworkPanel::OnRenderUI()
{
	static std::vector<const char*> modes =
	{
		"Standalone",
		"DedicatedServer",
		"ListenServer",
		"Client"
	};
	int current = (int)m_Settings.NetMode;
	if (UI::Combo("Net mode", current, modes.data(), modes.size()))
	{
		m_Settings.NetMode = (ENetDriverMode)current;
	}

	UI::Field("IP", m_Settings.Ip);

	int port = (int)m_Settings.Port;
	if (UI::InputDigits("Port", port, 5))
		m_Settings.Port = (uint32_t)port;

	if (!m_pDriver)
		return;

	if (m_pDriver->IsServer())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.9f, 0.1f, 1.0f));
		ImGui::Text("Running");
		ImGui::PopStyleColor();

		m_pDriver->ForeachConnection([](NetConnection* pConnection)
			{
				std::ostringstream ss;
				ss << "[client " << pConnection->GetId() << "] ping : " << (int)pConnection->GetPing();
				ImGui::Text(ss.str().c_str());
			});
	}
	else if (m_pDriver->IsClient())
	{
		if (m_pDriver->GetConnectionCount() == 0)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.0f, 0.1f, 1.0f));
			ImGui::Text("Not connected");
			ImGui::PopStyleColor();
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.9f, 0.1f, 1.0f));
			ImGui::Text("Connected");
			ImGui::PopStyleColor();

			std::ostringstream ss;
			ss << "ping : " << (int)m_pDriver->GetConnection(1)->GetPing();
			ImGui::Text(ss.str().c_str());
		}
	}
}
