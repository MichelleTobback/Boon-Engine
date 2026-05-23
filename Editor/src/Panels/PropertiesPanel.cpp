#include "Panels/PropertiesPanel.h"

#include <Component/CameraComponent.h>
#include <Component/NameComponent.h>
#include <Component/SpriteRendererComponent.h>
#include <Component/SpriteAnimatorComponent.h>
#include <Component/BoxCollider2D.h>

#include <Asset/AssetLibrary.h>
#include <Asset/SpriteAtlasAsset.h>

#include "Renderer/Texture.h"

#include <Core/ServiceLocator.h>
#include <Core/EditorContext.h>
#include <Command/EditorCommandQueue.h>
#include <Command/PropertyCommand.h>
#include <Command/SceneCommands.h>

#include <unordered_map>

using namespace BoonEditor;

BoonEditor::PropertiesPanel::PropertiesPanel(const std::string& name, EditorContext* pContext, GameObjectContext* pGameObject)
	: EditorPanel(name, pContext), m_pGameObjectContext{pGameObject}
{
	
}

void BoonEditor::PropertiesPanel::OnRenderUI()
{
	if (!m_pGameObjectContext || !m_pGameObjectContext->IsValid() || !m_pGameObjectContext->Get().IsValid())
	{
		ImGui::TextDisabled("%s  No GameObject selected", ICON_FA_CUBE);
		return;
	}

	GameObject& gameObject = m_pGameObjectContext->Get();

	if (!gameObject.HasComponent<NameComponent>())
		gameObject.AddComponent<NameComponent>();

	NameComponent& nameComponent = gameObject.GetComponent<NameComponent>();

	ImVec4 cardBg = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
	cardBg.x *= 0.72f;
	cardBg.y *= 0.72f;
	cardBg.z *= 0.72f;

	ImVec4 headerBg = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
	headerBg.x *= 0.82f;
	headerBg.y *= 0.82f;
	headerBg.z *= 0.82f;

	ImGui::PushStyleColor(ImGuiCol_ChildBg, cardBg);
	ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Border));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, ImGui::GetStyle().WindowRounding);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	if (ImGui::BeginChild("##PropertiesCard", ImVec2(0.0f, 0.0f), true))
	{
		ImDrawList* drawList = ImGui::GetWindowDrawList();

		const ImVec2 cardMin = ImGui::GetWindowPos();
		const float cardWidth = ImGui::GetWindowWidth();
		const float headerHeight = 52.0f;

		drawList->AddRectFilled(
			cardMin,
			ImVec2(cardMin.x + cardWidth, cardMin.y + headerHeight),
			ImGui::GetColorU32(headerBg),
			ImGui::GetStyle().WindowRounding,
			ImDrawFlags_RoundCornersTop);

		drawList->AddLine(
			ImVec2(cardMin.x, cardMin.y + headerHeight),
			ImVec2(cardMin.x + cardWidth, cardMin.y + headerHeight),
			ImGui::GetColorU32(ImGuiCol_Border));

		ImGui::SetCursorPos(ImVec2(12.0f, 10.0f));

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 6.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 6.0f));

		ImGui::AlignTextToFramePadding();
		ImGui::Text("%s", ICON_FA_CUBE);

		ImGui::SameLine();

		char nameBuffer[64]{};
		strcpy_s(nameBuffer, sizeof(nameBuffer), nameComponent.Name.c_str());

		ImGui::SetNextItemWidth(std::max(180.0f, ImGui::GetContentRegionAvail().x - 150.0f));

		ImVec4 bg = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
		bg.x *= 0.65f;
		bg.y *= 0.65f;
		bg.z *= 0.65f;

		ImGui::PushStyleColor(ImGuiCol_FrameBg, bg);

		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

		if (ImGui::InputText("##gameObjectName", nameBuffer, sizeof(nameBuffer)))
			nameComponent.Name = std::string(nameBuffer);

		ImGui::PopStyleVar();
		ImGui::PopStyleColor(1);

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));

		if (ImGui::Button(ICON_FA_PLUS, ImVec2(30.0f, 30.0f)))
			ImGui::OpenPopup("AddComponentPopup");

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Add component");

		ImGui::PopStyleColor();

		if (ImGui::BeginPopup("AddComponentPopup"))
		{
			ImGui::TextDisabled("%s  Add Component", ICON_FA_PUZZLE_PIECE);
			ImGui::Separator();

			BClassRegistry::Get().ForEach(
				[this](const BClass& cls)
				{
					if (m_pGameObjectContext->Get().HasComponentByClass(&cls))
						return;

					if (ImGui::MenuItem(cls.name.c_str()))
						m_pGameObjectContext->Get().AddComponentFromClass(&cls);
				});

			ImGui::EndPopup();
		}

		ImGui::PopStyleVar(2);

		ImGui::SetCursorPos(ImVec2(10.0f, headerHeight + 10.0f));

		ImGui::BeginChild("##PropertiesScroll", ImVec2(0.0f, 0.0f), false);

		RenderComponentNode<TransformComponent>( "Transform", false, [this](TransformComponent& transform)
			{
				BClassRegistry& reg = BClassRegistry::Get();
				BClass* cls = reg.Find<TransformComponent>();

				glm::vec3 value = transform.GetLocalPosition();
				UI::PropertyResult result = UI::Float3Control("Position", value);

				if (result)
				{
					if (result.Committed)
					{
						GetContext().GetCommandQueue()->Push<TransformValueCommand>(
							m_pGameObjectContext->Get(),
							result.OldValue.AsVec3(),
							value,
							[](TransformComponent& transform, const glm::vec3& value)
							{
								transform.SetLocalPosition(value);
							});
					}

					transform.SetLocalPosition(value);
				}

				value = transform.GetLocalEulerRotation();
				result = UI::Float3Control("Rotation", value);

				if (result)
				{
					if (result.Committed)
					{
						GetContext().GetCommandQueue()->Push<TransformValueCommand>(
							m_pGameObjectContext->Get(),
							result.OldValue.AsVec3(),
							value,
							[](TransformComponent& transform, const glm::vec3& value)
							{
								transform.SetLocalRotation(value);
							});
					}

					transform.SetLocalRotation(value);
				}

				value = transform.GetLocalScale();
				result = UI::Float3Control("Scale", value);

				if (result)
				{
					if (result.Committed)
					{
						GetContext().GetCommandQueue()->Push<TransformValueCommand>(
							m_pGameObjectContext->Get(),
							result.OldValue.AsVec3(),
							value,
							[](TransformComponent& transform, const glm::vec3& value)
							{
								transform.SetLocalScale(value);
							});
					}

					transform.SetLocalScale(value);
				}
			}, ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT);

		RenderComponentNode<CameraComponent>("Camera", true, [](CameraComponent& cameraComp)
			{
				Camera& camera = cameraComp.Camera;

				int projectionType = static_cast<int>(camera.GetProjectionType());
				const char* projectionTypes[] = { "Perspective", "Orthographic" };

				if (UI::Combo("Mode", projectionType, projectionTypes, IM_ARRAYSIZE(projectionTypes)))
					camera.SetProjectionType(static_cast<Camera::ProjectionType>(projectionType));

				switch (camera.GetProjectionType())
				{
				case Camera::ProjectionType::Perspective:
				{
					float fov = camera.GetFov();
					if (UI::SliderFloat("FOV", fov, 1.0f, 180.0f))
						camera.SetFov(fov);
					break;
				}

				case Camera::ProjectionType::Orthographic:
				{
					float size = camera.GetSize();
					if (UI::SliderFloat("Size", size, 1.0f, 100.0f))
						camera.SetSize(size);
					break;
				}
				}

				float value = camera.GetNear();
				if (UI::DragFloat("Near", value))
					camera.SetNear(value);

				value = camera.GetFar();
				if (UI::DragFloat("Far", value))
					camera.SetFar(value);
			}, ICON_FA_CAMERA);

		BClassRegistry& reg = BClassRegistry::Get();
		reg.ForEach([this](BClass& cls)
			{
				if (m_pGameObjectContext->Get().HasComponentByClass(&cls))
					RenderComponentNode(&cls);
			});

		ImGui::EndChild();
	}

	ImGui::EndChild();

	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(2);
}

inline void PropertiesPanel::RenderComponentNode(
	BClass* cls,
	const std::function<void()>& fn)
{
	GameObject& owner = m_pGameObjectContext->Get();

	if (!cls)
		return;

	if (!owner.HasComponentByClass(cls))
		return;

	if (cls->HasMeta("HideInInspector"))
		return;

	static std::unordered_map<std::string, bool> s_OpenStates;

	const std::string id =
		cls->name + std::to_string(owner.GetUUID());

	const std::string name =
		cls->HasMeta("Name")
		? cls->GetMeta("Name").value()
		: cls->name;

	bool& open = s_OpenStates[id];

	if (s_OpenStates.find(id) == s_OpenStates.end())
		open = true;

	const char* icon = ICON_FA_CUBE;

	if (cls->name == "TransformComponent")
		icon = ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT;
	else if (cls->name == "CameraComponent")
		icon = ICON_FA_CAMERA;
	else if (cls->name == "SpriteRendererComponent")
		icon = ICON_FA_IMAGE;
	else if (cls->name == "SpriteAnimatorComponent")
		icon = ICON_FA_FILM;
	else if (cls->name == "BoxCollider2D")
		icon = ICON_FA_DRAW_POLYGON;

	bool hasVisibleProperties = false;

	cls->ForEachProperty(
		[&hasVisibleProperties](const BProperty&)
		{
			hasVisibleProperties = true;
		});

	const bool canExpand =
		hasVisibleProperties ||
		static_cast<bool>(fn);

	RenderComponentNode_Internal(
		id,
		name,
		icon,
		canExpand,
		true,
		open,
		[&, cls]()
		{
			void* pInstance =
				owner.GetComponentByClass(cls);

			if (pInstance)
			{
				cls->ForEachProperty(
					[pInstance, this, owner, cls](const BProperty& prop)
					{
						if (prop.IsVariant())
						{
							UI::PropertyResult result =
								UI::Property(prop, pInstance);

							if (result.Committed)
							{
								Variant newVal =
									cls->GetValue(
										pInstance,
										prop.name.c_str());

								GetContext().GetCommandQueue()->Push<SetComponentPropertyCommand>(
									owner,
									cls,
									&prop,
									result.OldValue,
									newVal);
							}
						}
						else
						{
							UI::Property(prop, pInstance);
						}
					});
			}

			if (fn)
				fn();
		},
		[&, cls]()
		{
			owner.RemoveComponentByClass(cls);
		});
}

void PropertiesPanel::RenderComponentNode_Internal(
	const std::string& id,
	const std::string& name,
	const char* icon,
	bool canExpand,
	bool canBeRemoved,
	bool& open,
	const std::function<void()>& drawBody,
	const std::function<void()>& onRemove)
{
	ImGui::Dummy(ImVec2(0.0f, 2.0f));

	const float headerHeight = 32.0f;
	const float radius = 8.0f;
	const float buttonSize = 28.0f;

	ImVec2 min = ImGui::GetCursorScreenPos();
	ImVec2 avail = ImGui::GetContentRegionAvail();
	ImVec2 max(min.x + avail.x, min.y + headerHeight);

	const float menuX = max.x - buttonSize - 8.0f;
	const float menuY = min.y + (headerHeight - buttonSize) * 0.5f;

	const ImRect headerRect(min, max);
	const ImRect menuRect(
		ImVec2(menuX, menuY),
		ImVec2(menuX + buttonSize, menuY + buttonSize));

	const bool hovered =
		ImGui::IsMouseHoveringRect(headerRect.Min, headerRect.Max);

	const bool menuHovered =
		ImGui::IsMouseHoveringRect(menuRect.Min, menuRect.Max);

	if (canExpand && hovered && !menuHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		open = !open;

	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImVec4 headerBg = hovered
		? ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered)
		: ImGui::GetStyleColorVec4(ImGuiCol_Header);

	headerBg.w *= hovered ? 0.75f : 0.55f;

	ImVec4 iconColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);

	if (!canExpand)
	{
		headerBg.w *= 0.55f;
		iconColor.w *= 0.65f;
	}

	drawList->AddRectFilled(min, max, ImGui::GetColorU32(headerBg), radius);
	drawList->AddRect(min, max, ImGui::GetColorU32(ImGuiCol_Border), radius);

	const float textY =
		min.y + (headerHeight - ImGui::GetTextLineHeight()) * 0.5f;

	drawList->AddText(
		ImVec2(min.x + 12.0f, textY),
		ImGui::GetColorU32(iconColor),
		icon ? icon : ICON_FA_CUBE);

	drawList->AddText(
		ImVec2(min.x + 38.0f, textY),
		ImGui::GetColorU32(ImGuiCol_Text),
		name.c_str());

	bool removeComponent = false;

	ImGui::SetCursorScreenPos(ImVec2(menuX, menuY));
	ImGui::PushID(id.c_str());

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));

	if (canBeRemoved && ImGui::Button(ICON_FA_ELLIPSIS_VERTICAL, ImVec2(buttonSize, buttonSize)))
		ImGui::OpenPopup("ComponentSettings");

	ImGui::PopStyleColor();

	if (ImGui::BeginPopup("ComponentSettings"))
	{
		if (canBeRemoved && ImGui::MenuItem(ICON_FA_TRASH_CAN "  Remove Component"))
			removeComponent = true;

		ImGui::EndPopup();
	}

	ImGui::PopID();

	ImGui::SetCursorScreenPos(ImVec2(min.x, max.y));

	if (canExpand && open)
	{
		ImVec4 bodyBg = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
		bodyBg.x *= 0.85f;
		bodyBg.y *= 0.85f;
		bodyBg.z *= 0.85f;

		const float estimatedBodyHeight = 160.0f;

		drawList->AddRectFilled(
			ImVec2(min.x, max.y - 1.0f),
			ImVec2(max.x, max.y + estimatedBodyHeight),
			ImGui::GetColorU32(bodyBg),
			radius,
			ImDrawFlags_RoundCornersBottom);

		ImGui::SetCursorScreenPos(ImVec2(min.x + 10.0f, max.y + 8.0f));

		ImGui::BeginGroup();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 4.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 5.0f));

		if (drawBody)
			drawBody();

		ImGui::PopStyleVar(2);

		ImGui::EndGroup();

		ImVec2 bodyMax = ImGui::GetItemRectMax();

		drawList->AddRect(
			ImVec2(min.x, min.y),
			ImVec2(max.x, bodyMax.y + 10.0f),
			ImGui::GetColorU32(ImGuiCol_Border),
			radius);

		ImGui::SetCursorScreenPos(ImVec2(min.x, bodyMax.y + 4.0f));
		ImGui::Dummy(ImVec2(0.0f, 1.0f));
	}
	else
	{
		ImGui::SetCursorScreenPos(ImVec2(min.x, max.y - 8.0f));
		ImGui::Dummy(ImVec2(0.0f, 0.0f));
	}

	if (removeComponent && onRemove)
		onRemove();
}
