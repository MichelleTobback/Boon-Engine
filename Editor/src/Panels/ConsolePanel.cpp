#include "Panels/ConsolePanel.h"
#include <imgui.h>
#include <UI/IconsFontAwesome7.h>

#include "Core/EditorLogSink.h"
#include "BoonDebug/Logger.h"
#include "Core/ServiceLocator.h"

using namespace BoonEditor;

ConsolePanel::ConsolePanel(const std::string& name, EditorContext* pContext)
	: EditorPanel(name, pContext), m_pSink{ std::make_shared<EditorLogSink>(this) }
{
	Boon::ServiceLocator::Get<Boon::Logger>().AddSink(m_pSink);
}

BoonEditor::ConsolePanel::~ConsolePanel()
{
	Boon::ServiceLocator::Get<Boon::Logger>().RemoveSink(m_pSink.get());
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

	auto GetIconForLevel = [](int level) -> const char*
		{
			switch (level)
			{
			case 1: return ICON_FA_TRIANGLE_EXCLAMATION;
			case 2: return ICON_FA_CIRCLE_XMARK;
			default: return ICON_FA_CIRCLE_INFO;
			}
		};

	auto Accent = [](float alpha) -> ImU32
		{
			ImVec4 c = ImGui::GetStyleColorVec4(ImGuiCol_CheckMark);
			c.w *= alpha;
			return ImGui::GetColorU32(c);
		};

	auto GetPrefixColorForLevel = [&Accent](int level) -> ImU32
		{
			switch (level)
			{
			case 1:
				return ImGui::GetColorU32(ImVec4(1.0f, 0.72f, 0.28f, 1.0f));
			case 2:
				return ImGui::GetColorU32(ImVec4(1.0f, 0.38f, 0.42f, 1.0f));
			default:
				return Accent(0.95f);
			}
		};

	auto GetBadgeBgForLevel = [&Accent](int level) -> ImU32
		{
			switch (level)
			{
			case 1:
				return ImGui::GetColorU32(ImVec4(1.0f, 0.72f, 0.28f, 0.14f));
			case 2:
				return ImGui::GetColorU32(ImVec4(1.0f, 0.25f, 0.32f, 0.16f));
			default:
				return Accent(0.12f);
			}
		};

	auto AppendSelectedLinesToClipboard =
		[&NormalizeSelection](const std::vector<RenderLine>& lines, int start, int end)
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

	size_t messageCount = 0;
	{
		std::scoped_lock lock(m_Mutex);
		messageCount = m_Messages.size();
	}

	bool canCopySelection =
		s_HasSelection &&
		s_SelectionStart >= 0 &&
		s_SelectionEnd >= 0;

	bool copySelectionPressed = false;

	std::vector<ConsoleMessage> messages;
	{
		std::scoped_lock lock(m_Mutex);
		messages = m_Messages;
	}

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
			line.IsContinuation = lineIndex != 0;

			renderLines.push_back(std::move(line));
		}
	}

	ImVec4 cardBg = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
	cardBg.x *= 0.72f;
	cardBg.y *= 0.72f;
	cardBg.z *= 0.72f;

	ImGui::PushStyleColor(ImGuiCol_ChildBg, cardBg);
	ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Border));

	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, ImGui::GetStyle().WindowRounding);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	if (ImGui::BeginChild(
		"##ConsoleCard",
		ImVec2(0.0f, 0.0f),
		true,
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImDrawList* cardDrawList = ImGui::GetWindowDrawList();

		const ImVec2 cardMin = ImGui::GetWindowPos();
		const float cardWidth = ImGui::GetWindowWidth();
		const float headerHeight = 44.0f;

		ImVec4 headerBg = ImGui::GetStyleColorVec4(ImGuiCol_Header);

		cardDrawList->AddRectFilled(
			cardMin,
			ImVec2(cardMin.x + cardWidth, cardMin.y + headerHeight),
			ImGui::GetColorU32(headerBg),
			ImGui::GetStyle().WindowRounding,
			ImDrawFlags_RoundCornersTop);

		cardDrawList->AddLine(
			ImVec2(cardMin.x, cardMin.y + headerHeight),
			ImVec2(cardMin.x + cardWidth, cardMin.y + headerHeight),
			ImGui::GetColorU32(ImGuiCol_Border));

		ImGui::SetCursorPos(ImVec2(10.0f, 8.0f));

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 5.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));

		ImGui::AlignTextToFramePadding();
		ImGui::Text("%s  Console", ICON_FA_TERMINAL);

		ImGui::SameLine();
		ImGui::TextDisabled("| %zu messages", messageCount);

		ImGui::SameLine();

		const float rightWidth = 310.0f;
		const float cursorX = ImGui::GetCursorPosX();
		const float availableX = ImGui::GetContentRegionAvail().x;

		if (availableX > rightWidth)
			ImGui::SetCursorPosX(cursorX + availableX - rightWidth);

		if (ImGui::Button(ICON_FA_TRASH_CAN, ImVec2(30.0f, 28.0f)))
		{
			Clear();
			s_HasSelection = false;
			s_SelectionStart = -1;
			s_SelectionEnd = -1;
		}

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Clear console");

		ImGui::SameLine();

		if (ImGui::Button(ICON_FA_COPY, ImVec2(30.0f, 28.0f)))
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

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Copy all");

		ImGui::SameLine();

		if (!canCopySelection)
			ImGui::BeginDisabled();

		if (ImGui::Button(ICON_FA_CLIPBOARD, ImVec2(30.0f, 28.0f)))
			copySelectionPressed = true;

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Copy selection");

		if (!canCopySelection)
			ImGui::EndDisabled();

		ImGui::SameLine(0.0f, 10.0f);
		ImGui::Checkbox("Auto-scroll", &m_AutoScroll);

		ImGui::PopStyleVar(2);

		if (copySelectionPressed && canCopySelection)
			AppendSelectedLinesToClipboard(renderLines, s_SelectionStart, s_SelectionEnd);

		ImGui::SetCursorPos(ImVec2(0.0f, headerHeight));

		if (ImGui::BeginChild(
			"ConsoleRegion",
			ImVec2(0.0f, 0.0f),
			false,
			ImGuiWindowFlags_HorizontalScrollbar))
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImGuiIO& io = ImGui::GetIO();

			const ImVec2 avail = ImGui::GetContentRegionAvail();
			const float scrollX = ImGui::GetScrollX();
			const float scrollY = ImGui::GetScrollY();

			const float lineHeight = ImGui::GetTextLineHeightWithSpacing() + 4.0f;
			const float rowPaddingX = 8.0f;
			const float badgeWidth = 76.0f;
			const float badgeHeight = ImGui::GetTextLineHeight() + 6.0f;
			const float badgeOffsetY = (lineHeight - badgeHeight) * 0.5f;
			const float textOffsetY = (lineHeight - ImGui::GetTextLineHeight()) * 0.5f;
			const float textStartX = rowPaddingX + badgeWidth + 10.0f;

			float longestLineWidth = 0.0f;

			for (const auto& line : renderLines)
			{
				std::string displayText;

				if (!line.IsContinuation)
				{
					displayText += line.Prefix;
					displayText += " ";
				}
				else
				{
					displayText += "       ";
				}

				displayText += line.Text;

				longestLineWidth =
					std::max(longestLineWidth, ImGui::CalcTextSize(displayText.c_str()).x);
			}

			const float contentHeight =
				renderLines.empty()
				? avail.y
				: static_cast<float>(renderLines.size()) * lineHeight;

			const float contentWidth =
				std::max(avail.x, textStartX + longestLineWidth + 20.0f);

			ImGui::InvisibleButton(
				"ConsoleCanvas",
				ImVec2(contentWidth, contentHeight),
				ImGuiButtonFlags_MouseButtonLeft);

			const bool canvasHovered = ImGui::IsItemHovered();
			const ImVec2 canvasMin = ImGui::GetItemRectMin();

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

			if (s_HasSelection &&
				(ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) ||
					ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) &&
				ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_C))
			{
				AppendSelectedLinesToClipboard(renderLines, s_SelectionStart, s_SelectionEnd);
			}

			int visibleStart =
				std::max(0, static_cast<int>(scrollY / lineHeight));

			int visibleEnd =
				std::min(
					static_cast<int>(renderLines.size()),
					static_cast<int>((scrollY + avail.y) / lineHeight) + 2);

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

				if ((i % 2) == 0)
				{
					ImVec4 stripe = ImGui::GetStyleColorVec4(ImGuiCol_Text);
					stripe.w = 0.025f;

					drawList->AddRectFilled(
						rowMin,
						rowMax,
						ImGui::GetColorU32(stripe));
				}

				if (s_HasSelection && i >= selStart && i <= selEnd)
				{
					ImVec4 base = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
					base.w = 0.35f;

					drawList->AddRectFilled(
						rowMin,
						rowMax,
						ImGui::GetColorU32(base));
				}

				const ImU32 badgeTextColor = GetPrefixColorForLevel(line.Level);
				const ImU32 badgeBgColor = GetBadgeBgForLevel(line.Level);

				const float badgeX = rowMinX + rowPaddingX;
				const float badgeY = y + badgeOffsetY;

				const ImVec2 badgeMin(badgeX, badgeY);
				const ImVec2 badgeMax(badgeX + badgeWidth, badgeY + badgeHeight);

				if (!line.IsContinuation)
				{
					drawList->AddRectFilled(badgeMin, badgeMax, badgeBgColor, 6.0f);
					drawList->AddRect(badgeMin, badgeMax, badgeTextColor, 6.0f, 0, 1.0f);

					std::string badgeText =
						std::string(GetIconForLevel(line.Level)) +
						" " +
						line.Prefix;

					ImVec2 prefixSize =
						ImGui::CalcTextSize(badgeText.c_str());

					ImVec2 prefixPos(
						badgeX + (badgeWidth - prefixSize.x) * 0.5f,
						badgeY + (badgeHeight - prefixSize.y) * 0.5f);

					drawList->AddText(
						prefixPos,
						badgeTextColor,
						badgeText.c_str());
				}

				const ImU32 textColor =
					line.Level == 2
					? ImGui::GetColorU32(ImVec4(1.0f, 0.86f, 0.86f, 1.0f))
					: line.Level == 1
					? ImGui::GetColorU32(ImVec4(1.0f, 0.94f, 0.76f, 1.0f))
					: ImGui::GetColorU32(ImGuiCol_Text);

				ImVec2 textPos(rowMinX + textStartX, y + textOffsetY);

				drawList->AddText(
					textPos,
					textColor,
					line.Text.c_str());
			}

			if (m_AutoScroll &&
				(m_ScrollToBottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
			{
				ImGui::SetScrollY(ImGui::GetScrollMaxY());
				m_ScrollToBottom = false;
			}
		}

		ImGui::EndChild();
	}

	ImGui::EndChild();

	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(2);
}