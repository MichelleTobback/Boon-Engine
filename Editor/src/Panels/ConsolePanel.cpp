#include "Panels/ConsolePanel.h"
#include <imgui.h>
#include <UI/IconsFontAwesome7.h>

#include "Core/EditorLogSink.h"
#include "BoonDebug/Logger.h"
#include "Core/ServiceLocator.h"

using namespace BoonEditor;

ConsolePanel::ConsolePanel(EditorContext* pContext, const std::string& name)
	: EditorPanel(pContext, name), m_pSink{ std::make_shared<EditorLogSink>(this) }
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

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	if (ImGui::BeginChild(
		"##ConsoleRoot",
		ImVec2(0, 0),
		true,
		ImGuiWindowFlags_NoScrollbar))
	{
		const float toolbarHeight = 42.0f;

		ImDrawList* drawList = ImGui::GetWindowDrawList();

		ImVec2 winPos = ImGui::GetWindowPos();
		ImVec2 winSize = ImGui::GetWindowSize();

		drawList->AddRectFilled(
			winPos,
			ImVec2(winPos.x + winSize.x, winPos.y + toolbarHeight),
			ImGui::GetColorU32(ImGuiCol_TitleBg),
			ImGui::GetStyle().WindowRounding,
			ImDrawFlags_RoundCornersTop);

		ImGui::SetCursorPos(ImVec2(10.0f, 8.0f));

		ImGui::Text("%s Console", ICON_FA_TERMINAL);

		ImGui::SameLine();
		ImGui::TextDisabled("| %zu messages", messages.size());

		ImGui::SameLine();

		if (ImGui::Button(ICON_FA_TRASH_CAN))
		{
			Clear();

			s_HasSelection = false;
			s_SelectionStart = -1;
			s_SelectionEnd = -1;
		}

		ImGui::SameLine();

		if (ImGui::Button(ICON_FA_COPY))
		{
			std::string allText;

			for (const auto& msg : messages)
			{
				allText += "[";
				allText += GetPrefixForLevel(msg.Level);
				allText += "] ";
				allText += msg.Text;
				allText += '\n';
			}

			ImGui::SetClipboardText(allText.c_str());
		}

		ImGui::SameLine();

		if (!s_HasSelection)
			ImGui::BeginDisabled();

		if (ImGui::Button(ICON_FA_CLIPBOARD))
		{
			AppendSelectedLinesToClipboard(
				renderLines,
				s_SelectionStart,
				s_SelectionEnd);
		}

		if (!s_HasSelection)
			ImGui::EndDisabled();

		ImGui::SameLine();
		ImGui::Checkbox("Auto-scroll", &m_AutoScroll);

		ImGui::SetCursorPos(ImVec2(0.0f, toolbarHeight));

		if (ImGui::BeginChild(
			"##ConsoleScroll",
			ImVec2(0, 0),
			false,
			ImGuiWindowFlags_HorizontalScrollbar))
		{
			ImGuiIO& io = ImGui::GetIO();
			ImDrawList* dl = ImGui::GetWindowDrawList();

			const float lineHeight =
				ImGui::GetTextLineHeightWithSpacing() + 4.0f;

			const float badgeWidth = 78.0f;
			const float rowPaddingX = 8.0f;
			const float textStartX = rowPaddingX + badgeWidth + 12.0f;

			float longestLineWidth = 0.0f;

			for (const auto& line : renderLines)
			{
				longestLineWidth =
					std::max(
						longestLineWidth,
						ImGui::CalcTextSize(line.Text.c_str()).x);
			}

			const float canvasWidth =
				textStartX + longestLineWidth + 40.0f;

			const float canvasHeight =
				std::max(
					ImGui::GetContentRegionAvail().y,
					lineHeight * static_cast<float>(renderLines.size()));

			ImGui::InvisibleButton(
				"##ConsoleCanvas",
				ImVec2(canvasWidth, canvasHeight),
				ImGuiButtonFlags_MouseButtonLeft);

			const ImVec2 canvasMin = ImGui::GetItemRectMin();

			auto MouseToLineIndex = [&](const ImVec2& mousePos) -> int
				{
					float localY = mousePos.y - canvasMin.y;

					if (localY < 0.0f)
						return -1;

					int idx = static_cast<int>(localY / lineHeight);

					if (idx < 0 || idx >= static_cast<int>(renderLines.size()))
						return -1;

					return idx;
				};

			if (ImGui::IsItemHovered() &&
				ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				int clickedLine = MouseToLineIndex(io.MousePos);

				if (clickedLine >= 0)
				{
					s_HasSelection = true;
					s_IsDraggingSelection = true;
					s_SelectionStart = clickedLine;
					s_SelectionEnd = clickedLine;
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

			int selStart = s_SelectionStart;
			int selEnd = s_SelectionEnd;

			if (s_HasSelection)
				NormalizeSelection(selStart, selEnd);

			for (int i = 0; i < static_cast<int>(renderLines.size()); ++i)
			{
				const auto& line = renderLines[i];

				const float y = canvasMin.y + i * lineHeight;

				const ImVec2 rowMin(canvasMin.x, y);
				const ImVec2 rowMax(canvasMin.x + canvasWidth, y + lineHeight);

				if ((i % 2) == 0)
				{
					ImVec4 stripe =
						ImGui::GetStyleColorVec4(ImGuiCol_Text);

					stripe.w = 0.025f;

					dl->AddRectFilled(
						rowMin,
						rowMax,
						ImGui::GetColorU32(stripe));
				}

				if (s_HasSelection &&
					i >= selStart &&
					i <= selEnd)
				{
					ImVec4 sel =
						ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);

					sel.w = 0.35f;

					dl->AddRectFilled(
						rowMin,
						rowMax,
						ImGui::GetColorU32(sel));
				}

				if (!line.IsContinuation)
				{
					const ImU32 badgeText =
						GetPrefixColorForLevel(line.Level);

					const ImU32 badgeBg =
						GetBadgeBgForLevel(line.Level);

					ImVec2 badgeMin(
						canvasMin.x + rowPaddingX,
						y + 2.0f);

					ImVec2 badgeMax(
						badgeMin.x + badgeWidth,
						badgeMin.y + ImGui::GetTextLineHeight() + 6.0f);

					dl->AddRectFilled(
						badgeMin,
						badgeMax,
						badgeBg,
						6.0f);

					dl->AddRect(
						badgeMin,
						badgeMax,
						badgeText,
						6.0f);

					std::string badge =
						std::string(GetIconForLevel(line.Level)) +
						" " +
						line.Prefix;

					ImVec2 textSize =
						ImGui::CalcTextSize(badge.c_str());

					dl->AddText(
						ImVec2(
							badgeMin.x + (badgeWidth - textSize.x) * 0.5f,
							badgeMin.y + 3.0f),
						badgeText,
						badge.c_str());
				}

				dl->AddText(
					ImVec2(canvasMin.x + textStartX, y + 3.0f),
					ImGui::GetColorU32(ImGuiCol_Text),
					line.Text.c_str());
			}

			if (m_AutoScroll && m_ScrollToBottom)
			{
				ImGui::SetScrollHereY(1.0f);
				m_ScrollToBottom = false;
			}
		}

		ImGui::EndChild();
	}

	ImGui::EndChild();

	ImGui::PopStyleVar();
}