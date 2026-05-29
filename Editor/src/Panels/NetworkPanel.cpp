#include "Panels/NetworkPanel.h"
#include "UI/UI.h"

#include <UI/IconsFontAwesome7.h>

#include <Networking/NetworkingSubsystem.h>

#include <sstream>
#include <Networking/NetDriver.h>
#include <Core/EditorContext.h>
#include <Core/EngineContext.h>
#include <Core/Application.h>

using namespace BoonEditor;
using namespace Boon;

namespace
{
	ImU32 Col(ImGuiCol idx)
	{
		return ImGui::GetColorU32(ImGui::GetStyleColorVec4(idx));
	}

	ImU32 Accent(float alpha = 1.0f)
	{
		ImVec4 c = ImGui::GetStyleColorVec4(ImGuiCol_CheckMark);
		c.w *= alpha;
		return ImGui::GetColorU32(c);
	}

	ImVec4 Darken(ImGuiCol idx, float amount)
	{
		ImVec4 c = ImGui::GetStyleColorVec4(idx);
		c.x *= amount;
		c.y *= amount;
		c.z *= amount;
		return c;
	}

	ImVec4 StatusColor(bool good)
	{
		if (good)
			return ImVec4(0.35f, 0.95f, 0.48f, 1.0f);

		return ImVec4(1.0f, 0.38f, 0.42f, 1.0f);
	}

	void DrawStatusPill(const char* icon, const char* text, bool good)
	{
		ImDrawList* drawList = ImGui::GetWindowDrawList();

		const ImVec4 color = StatusColor(good);
		ImVec4 bg = color;
		bg.w = 0.14f;

		const ImVec2 textSize = ImGui::CalcTextSize(text);
		const ImVec2 iconSize = ImGui::CalcTextSize(icon);

		const float height = 28.0f;
		const float width = iconSize.x + textSize.x + 26.0f;

		const ImVec2 min = ImGui::GetCursorScreenPos();
		const ImVec2 max(min.x + width, min.y + height);

		drawList->AddRectFilled(
			min,
			max,
			ImGui::GetColorU32(bg),
			height * 0.5f);

		drawList->AddRect(
			min,
			max,
			ImGui::GetColorU32(color),
			height * 0.5f,
			0,
			1.0f);

		drawList->AddText(
			ImVec2(min.x + 10.0f, min.y + (height - iconSize.y) * 0.5f),
			ImGui::GetColorU32(color),
			icon);

		drawList->AddText(
			ImVec2(min.x + 18.0f + iconSize.x, min.y + (height - textSize.y) * 0.5f),
			ImGui::GetColorU32(color),
			text);

		ImGui::Dummy(ImVec2(width, height));
	}

	void DrawInfoRow(const char* icon, const char* label, const char* value)
	{
		ImGui::AlignTextToFramePadding();

		ImGui::TextDisabled("%s", icon);

		ImGui::SameLine(28.0f);
		ImGui::TextDisabled("%s", label);

		ImGui::SameLine(130.0f);
		ImGui::TextUnformatted(value);
	}
}

NetworkPanel::NetworkPanel(EditorContext* pContext, const std::string& name)
	: EditorPanel(pContext, name)
{
	
}

void NetworkPanel::SetDriver(NetDriver* pDriver)
{
	m_pDriver = pDriver;
}

void BoonEditor::NetworkPanel::OnRenderUI()
{
	NetworkingSubsystem* netSubsystem = GetContext().GetEngineContext().TryGetSubsystem<NetworkingSubsystem>();

	static std::vector<const char*> modes =
	{
		"Standalone",
		"Dedicated Server",
		"Listen Server",
		"Client"
	};

	ImVec4 cardBg = Darken(ImGuiCol_ChildBg, 0.72f);
	ImVec4 headerBg = Darken(ImGuiCol_ChildBg, 0.82f);

	ImGui::PushStyleColor(ImGuiCol_ChildBg, cardBg);
	ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Border));

	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, ImGui::GetStyle().WindowRounding);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	if (ImGui::BeginChild(
		"##NetworkCard",
		ImVec2(0.0f, 0.0f),
		true,
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImDrawList* drawList = ImGui::GetWindowDrawList();

		const ImVec2 cardMin = ImGui::GetWindowPos();
		const float cardWidth = ImGui::GetWindowWidth();
		const float headerHeight = 44.0f;

		drawList->AddRectFilled(
			cardMin,
			ImVec2(cardMin.x + cardWidth, cardMin.y + headerHeight),
			ImGui::GetColorU32(headerBg),
			ImGui::GetStyle().WindowRounding,
			ImDrawFlags_RoundCornersTop);

		drawList->AddLine(
			ImVec2(cardMin.x, cardMin.y + headerHeight),
			ImVec2(cardMin.x + cardWidth, cardMin.y + headerHeight),
			Col(ImGuiCol_Border));

		ImGui::SetCursorPos(ImVec2(12.0f, 8.0f));

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 5.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));

		ImGui::AlignTextToFramePadding();
		ImGui::Text("%s  Network", ICON_FA_NETWORK_WIRED);

		ImGui::SameLine();

		NetworkSettings& settings = GetContext().GetEngineContext().ProjectConfig->Network;

		const char* modeText =
			modes[std::clamp(static_cast<int>(settings.NetMode), 0, static_cast<int>(modes.size()) - 1)];

		ImGui::TextDisabled("| %s", modeText);

		ImGui::PopStyleVar(2);

		ImGui::SetCursorPos(ImVec2(12.0f, headerHeight + 14.0f));

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 5.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));

		ImGui::BeginGroup();

		ImGui::TextDisabled("%s  Settings", ICON_FA_SLIDERS);
		ImGui::Separator();

		int current = static_cast<int>(settings.NetMode);

		if (UI::Combo("Net mode", current, modes.data(), static_cast<int>(modes.size())))
			settings.NetMode = static_cast<ENetDriverMode>(current);

		UI::Field("IP", settings.Ip);

		int port = static_cast<int>(settings.Port);
		if (UI::InputDigits("Port", port, 5))
			settings.Port = static_cast<uint32_t>(port);

		ImGui::EndGroup();

		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::TextDisabled("%s  Runtime", ICON_FA_SIGNAL);
		ImGui::Separator();

		if (!netSubsystem)
		{
			DrawStatusPill(ICON_FA_POWER_OFF, "Driver inactive", false);
			ImGui::TextDisabled("No active network driver is assigned.");
		}
		if (!netSubsystem->GetDriver().IsRunning())
		{
			DrawStatusPill(ICON_FA_POWER_OFF, "Driver inactive", false);
		}
		else if (netSubsystem->GetDriver().IsServer())
		{
			NetDriver& driver = netSubsystem->GetDriver();

			DrawStatusPill(ICON_FA_SERVER, "Running", true);

			ImGui::Spacing();

			int connectionCount = 0;

			driver.ForeachConnection(
				[&connectionCount](NetConnection* pConnection)
				{
					++connectionCount;

					std::ostringstream ss;
					ss << static_cast<int>(pConnection->GetPing()) << " ms";

					ImGui::PushID(pConnection->GetId());

					std::ostringstream clientLabel;
					clientLabel << "Client " << pConnection->GetId();

					DrawInfoRow(
						ICON_FA_USER,
						clientLabel.str().c_str(),
						ss.str().c_str());

					ImGui::PopID();
				});

			if (connectionCount == 0)
				ImGui::TextDisabled("No clients connected.");
		}
		else if (netSubsystem->GetDriver().IsClient())
		{
			NetDriver& driver = netSubsystem->GetDriver();

			if (driver.GetConnectionCount() == 0)
			{
				DrawStatusPill(ICON_FA_PLUG_CIRCLE_XMARK, "Not connected", false);
				ImGui::TextDisabled("Client mode is active, but no server connection exists.");
			}
			else
			{
				DrawStatusPill(ICON_FA_PLUG_CIRCLE_CHECK, "Connected", true);

				NetConnection* connection =
					driver.GetConnection(driver.GetLocalConnectionId());

				if (connection)
				{
					std::ostringstream ss;
					ss << static_cast<int>(connection->GetPing()) << " ms";

					DrawInfoRow(
						ICON_FA_GAUGE_HIGH,
						"Ping",
						ss.str().c_str());
				}
			}
		}

		ImGui::PopStyleVar(2);
	}

	ImGui::EndChild();

	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(2);
}