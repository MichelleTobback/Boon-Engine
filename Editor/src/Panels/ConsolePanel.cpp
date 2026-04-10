#include "Panels/ConsolePanel.h"
#include <imgui.h>

#include "Core/EditorLogSink.h"
#include "BoonDebug/Logger.h"
#include "Core/ServiceLocator.h"

using namespace BoonEditor;

ConsolePanel::ConsolePanel(const std::string& name, EditorContext* pContext)
	: EditorPanel(name, pContext)
{
	Boon::ServiceLocator::Get<Boon::Logger>().AddSink(std::make_shared<EditorLogSink>(this));
}

void ConsolePanel::AddMessage(const std::string& text, int level)
{
	std::scoped_lock lock(m_Mutex);
	m_Messages.push_back({ text, level });
	m_ScrollToBottom = true;
}

void ConsolePanel::Clear()
{
	std::scoped_lock lock(m_Mutex);
	m_Messages.clear();
}

void ConsolePanel::OnRenderUI()
{
	struct RenderLine
	{
		std::string Prefix;
		std::string Text;
		int Level{};
		size_t MessageIndex{};
		bool IsContinuation{};
	};

	// Persistent UI state for this widget
	static bool s_HasSelection = false;
	static int s_SelectionStart = -1;
	static int s_SelectionEnd = -1;
	static bool s_IsDraggingSelection = false;

	auto NormalizeSelection = [](int& a, int& b)
		{
			if (a > b)
				std::swap(a, b);
		};

	auto SplitLines = [](const std::string& text)
		{
			std::vector<std::string> lines;
			size_t start = 0;
			while (start <= text.size())
			{
				size_t end = text.find('\n', start);
				if (end == std::string::npos)
				{
					lines.emplace_back(text.substr(start));
					break;
				}

				lines.emplace_back(text.substr(start, end - start));
				start = end + 1;

				// Preserve trailing empty line if text ends with '\n'
				if (start == text.size())
				{
					lines.emplace_back("");
					break;
				}
			}

			if (lines.empty())
				lines.emplace_back("");

			return lines;
		};

	auto GetPrefixForLevel = [](int level) -> const char*
		{
			switch (level)
			{
			case 1: return "Warn";
			case 2: return "Error";
			default: return "Info";
			}
		};

	auto GetPrefixColorForLevel = [](int level) -> ImU32
		{
			switch (level)
			{
			case 1: return IM_COL32(255, 191, 64, 255);
			case 2: return IM_COL32(255, 95, 95, 255);
			default: return IM_COL32(170, 170, 170, 255);
			}
		};

	auto AppendSelectedLinesToClipboard = [&NormalizeSelection](const std::vector<RenderLine>& lines, int start, int end)
		{
			if (lines.empty() || start < 0 || end < 0)
				return;

			NormalizeSelection(start, end);

			start = std::max(0, start);
			end = std::min(end, static_cast<int>(lines.size()) - 1);

			std::string out;
			for (int i = start; i <= end; ++i)
			{
				const auto& line = lines[i];

				if (!line.IsContinuation)
				{
					out += "[";
					out += line.Prefix;
					out += "] ";
				}
				else
				{
					out += "       ";
				}

				out += line.Text;

				if (i < end)
					out += '\n';
			}

			ImGui::SetClipboardText(out.c_str());
		};

	// Toolbar
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 3.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));

	if (ImGui::SmallButton("Clear"))
	{
		Clear();
		s_HasSelection = false;
		s_SelectionStart = -1;
		s_SelectionEnd = -1;
	}

	ImGui::SameLine();

	if (ImGui::SmallButton("Copy All"))
	{
		std::string allText;
		{
			std::scoped_lock lock(m_Mutex);
			for (const auto& msg : m_Messages)
			{
				const char* prefix = GetPrefixForLevel(msg.Level);
				allText += "[";
				allText += prefix;
				allText += "] ";
				allText += msg.Text;
				allText += '\n';
			}
		}
		ImGui::SetClipboardText(allText.c_str());
	}

	ImGui::SameLine();

	bool canCopySelection = s_HasSelection && s_SelectionStart >= 0 && s_SelectionEnd >= 0;
	if (!canCopySelection)
		ImGui::BeginDisabled();

	if (ImGui::SmallButton("Copy Selection"))
	{
		// handled later after render lines are built
	}

	bool copySelectionPressed = ImGui::IsItemClicked();

	if (!canCopySelection)
		ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::Checkbox("Auto-scroll", &m_AutoScroll);

	ImGui::SameLine();
	ImGui::TextDisabled("|");

	size_t messageCount = 0;
	{
		std::scoped_lock lock(m_Mutex);
		messageCount = m_Messages.size();
	}

	ImGui::SameLine();
	ImGui::TextDisabled("%zu", messageCount);

	ImGui::PopStyleVar(3);

	ImGui::Separator();

	// Copy messages out before rendering
	std::vector<ConsoleMessage> messages;
	{
		std::scoped_lock lock(m_Mutex);
		messages = m_Messages;
	}

	// Flatten messages into render lines
	std::vector<RenderLine> renderLines;
	renderLines.reserve(messages.size());

	for (size_t msgIndex = 0; msgIndex < messages.size(); ++msgIndex)
	{
		const auto& msg = messages[msgIndex];
		const char* prefix = GetPrefixForLevel(msg.Level);

		auto split = SplitLines(msg.Text);
		for (size_t lineIndex = 0; lineIndex < split.size(); ++lineIndex)
		{
			RenderLine line{};
			line.Prefix = prefix;
			line.Text = split[lineIndex];
			line.Level = msg.Level;
			line.MessageIndex = msgIndex;
			line.IsContinuation = (lineIndex != 0);
			renderLines.push_back(std::move(line));
		}
	}

	if (copySelectionPressed && canCopySelection)
		AppendSelectedLinesToClipboard(renderLines, s_SelectionStart, s_SelectionEnd);

	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f, 0.105f, 0.11f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	if (ImGui::BeginChild("ConsoleRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar))
	{
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImGuiIO& io = ImGui::GetIO();

		const ImVec2 regionMin = ImGui::GetCursorScreenPos();
		const ImVec2 avail = ImGui::GetContentRegionAvail();
		const float scrollX = ImGui::GetScrollX();
		const float scrollY = ImGui::GetScrollY();

		const float lineHeight = ImGui::GetTextLineHeightWithSpacing() + 4.0f;
		const float rowPaddingX = 8.0f;
		const float badgeWidth = 52.0f;
		const float badgeHeight = ImGui::GetTextLineHeight() + 6.0f;
		const float badgeOffsetY = (lineHeight - badgeHeight) * 0.5f;
		const float textOffsetY = (lineHeight - ImGui::GetTextLineHeight()) * 0.5f;
		const float textStartX = rowPaddingX + badgeWidth + 10.0f;

		// Total virtual content size for scrolling
		float longestLineWidth = 0.0f;
		for (const auto& line : renderLines)
		{
			std::string displayText;
			if (!line.IsContinuation)
			{
				displayText = "[";
				displayText += line.Prefix;
				displayText += "] ";
			}
			else
			{
				displayText = "       ";
			}
			displayText += line.Text;

			longestLineWidth = std::max(longestLineWidth, ImGui::CalcTextSize(displayText.c_str()).x);
		}

		const float contentHeight = renderLines.empty() ? avail.y : (renderLines.size() * lineHeight);
		const float contentWidth = std::max(avail.x, textStartX + longestLineWidth + 20.0f);

		// Reserve the scrollable canvas
		ImGui::InvisibleButton("ConsoleCanvas", ImVec2(contentWidth, contentHeight), ImGuiButtonFlags_MouseButtonLeft);

		const bool canvasHovered = ImGui::IsItemHovered();
		const bool canvasActive = ImGui::IsItemActive();
		const ImVec2 canvasMin = ImGui::GetItemRectMin();

		// Mouse selection: line-based
		auto MouseToLineIndex = [&](const ImVec2& mousePos) -> int
			{
				float localY = mousePos.y - canvasMin.y + scrollY;
				if (localY < 0.0f)
					return -1;

				int idx = static_cast<int>(localY / lineHeight);
				if (idx < 0 || idx >= static_cast<int>(renderLines.size()))
					return -1;

				return idx;
			};

		if (canvasHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			int clickedLine = MouseToLineIndex(io.MousePos);
			if (clickedLine >= 0)
			{
				s_HasSelection = true;
				s_IsDraggingSelection = true;
				s_SelectionStart = clickedLine;
				s_SelectionEnd = clickedLine;
			}
			else
			{
				s_HasSelection = false;
				s_IsDraggingSelection = false;
				s_SelectionStart = -1;
				s_SelectionEnd = -1;
			}
		}

		if (s_IsDraggingSelection)
		{
			if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				int hoveredLine = MouseToLineIndex(io.MousePos);
				if (hoveredLine >= 0)
					s_SelectionEnd = hoveredLine;
			}
			else
			{
				s_IsDraggingSelection = false;
			}
		}

		// Ctrl+C for selection when the console region is hovered or focused
		if (s_HasSelection &&
			(ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) || ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) &&
			ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_C))
		{
			AppendSelectedLinesToClipboard(renderLines, s_SelectionStart, s_SelectionEnd);
		}

		// Clip visible lines
		int visibleStart = std::max(0, static_cast<int>(scrollY / lineHeight));
		int visibleEnd = std::min(static_cast<int>(renderLines.size()), static_cast<int>((scrollY + avail.y) / lineHeight) + 2);

		int selStart = s_SelectionStart;
		int selEnd = s_SelectionEnd;
		if (s_HasSelection && selStart >= 0 && selEnd >= 0)
			NormalizeSelection(selStart, selEnd);

		for (int i = visibleStart; i < visibleEnd; ++i)
		{
			const auto& line = renderLines[i];

			const float y = canvasMin.y + i * lineHeight - scrollY;
			const float rowMinX = canvasMin.x - scrollX;
			const float rowMaxX = rowMinX + contentWidth;
			const ImVec2 rowMin(rowMinX, y);
			const ImVec2 rowMax(rowMaxX, y + lineHeight);

			// Alternating background
			if ((i % 2) == 0)
			{
				drawList->AddRectFilled(rowMin, rowMax, IM_COL32(255, 255, 255, 8));
			}

			// Selection background
			if (s_HasSelection && i >= selStart && i <= selEnd)
			{
				ImVec4 base = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
				ImU32 selectionColor = ImGui::ColorConvertFloat4ToU32(
					ImVec4(base.x * 0.6f, base.y * 0.6, base.z * 0.6, 0.35f)
				);

				drawList->AddRectFilled(rowMin, rowMax, selectionColor);
			}

			// Badge
			const ImU32 badgeTextColor = GetPrefixColorForLevel(line.Level);
			const ImU32 badgeBgColor =
				(line.Level == 2) ? IM_COL32(110, 35, 35, 180) :
				(line.Level == 1) ? IM_COL32(110, 85, 30, 180) :
				IM_COL32(70, 70, 70, 160);

			const float badgeX = rowMinX + rowPaddingX;
			const float badgeY = y + badgeOffsetY;
			const ImVec2 badgeMin(badgeX, badgeY);
			const ImVec2 badgeMax(badgeX + badgeWidth, badgeY + badgeHeight);

			if (!line.IsContinuation)
			{
				drawList->AddRectFilled(badgeMin, badgeMax, badgeBgColor, 3.0f);

				ImVec2 prefixSize = ImGui::CalcTextSize(line.Prefix.c_str());
				ImVec2 prefixPos(
					badgeX + (badgeWidth - prefixSize.x) * 0.5f,
					badgeY + (badgeHeight - prefixSize.y) * 0.5f
				);
				drawList->AddText(prefixPos, badgeTextColor, line.Prefix.c_str());
			}

			// Message text
			const ImU32 textColor =
				(line.Level == 2) ? IM_COL32(255, 225, 225, 255) :
				(line.Level == 1) ? IM_COL32(255, 245, 210, 255) :
				IM_COL32(230, 230, 230, 255);

			ImVec2 textPos(rowMinX + textStartX, y + textOffsetY);
			drawList->AddText(textPos, textColor, line.Text.c_str());
		}

		// Auto-scroll
		if (m_AutoScroll && (m_ScrollToBottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
		{
			ImGui::SetScrollY(ImGui::GetScrollMaxY());
			m_ScrollToBottom = false;
		}
	}
	ImGui::EndChild();

	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
}