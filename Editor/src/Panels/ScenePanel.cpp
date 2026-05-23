#include "Panels/ScenePanel.h"

#include <Component/UUIDComponent.h>
#include <Component/NameComponent.h>
#include <Component/SceneComponent.h>
#include <Component/SpriteRendererComponent.h>
#include <Component/CameraComponent.h>
#include <Component/Rigidbody2D.h>

#include <UI/IconsFontAwesome7.h>

#include <imgui.h>

using namespace BoonEditor;

namespace
{
	constexpr const char* GameObjectPayloadName = "BOON_GAMEOBJECT_PAYLOAD";

	static const char* GetGameObjectIcon(GameObject gameObject)
	{
		if (gameObject.HasComponent<CameraComponent>())
			return ICON_FA_VIDEO;

		if (gameObject.HasComponent<Rigidbody2D>())
			return ICON_FA_CUBES_STACKED;

		if (gameObject.HasComponent<SpriteRendererComponent>())
			return ICON_FA_IMAGE;

		return ICON_FA_CUBE;
	}

	static ImVec4 GetMutedTextColor()
	{
		ImVec4 color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
		color.w *= 0.55f;
		return color;
	}
}

ScenePanel::ScenePanel(
	const std::string& name,
	EditorContext* pContext,
	SceneContext* pScene,
	GameObjectContext* pSelectedGameObject)
	: EditorPanel(name, pContext)
	, m_pSceneContext{ pScene }
	, m_pSelectionContext{ pSelectedGameObject }
{
}

void ScenePanel::OnRenderUI()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(7.0f, 5.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 5.0f));

	ImVec4 childBg = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
	ImVec4 border = ImGui::GetStyleColorVec4(ImGuiCol_Border);
	ImVec4 menuBg = ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg);
	ImVec4 header = ImGui::GetStyleColorVec4(ImGuiCol_Header);
	ImVec4 headerHovered = ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered);
	ImVec4 headerActive = ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive);
	ImVec4 textDisabled = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);

	childBg.x *= 0.52f;
	childBg.y *= 0.52f;
	childBg.z *= 0.52f;

	menuBg.x *= 0.82f;
	menuBg.y *= 0.82f;
	menuBg.z *= 0.82f;

	border.w *= 1.35f;

	ImGui::PushStyleColor(ImGuiCol_ChildBg, childBg);
	ImGui::PushStyleColor(ImGuiCol_Border, border);

	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	if (ImGui::BeginChild(
		"##SceneHierarchyFrame",
		ImVec2(0.0f, 0.0f),
		true,
		ImGuiWindowFlags_MenuBar))
	{
		if (ImGui::BeginMenuBar())
		{
			ImGui::TextUnformatted("Scene Hierarchy");

			ImGui::SameLine();

			float menuWidth =
				ImGui::CalcTextSize(ICON_FA_PLUS).x +
				ImGui::GetStyle().FramePadding.x * 2.0f +
				12.0f;

			float rightAlign =
				ImGui::GetWindowWidth() -
				menuWidth -
				ImGui::GetStyle().WindowPadding.x * 2.0f;

			if (rightAlign > ImGui::GetCursorPosX())
				ImGui::SetCursorPosX(rightAlign);

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 6.0f));

			if (ImGui::BeginMenu(ICON_FA_PLUS))
			{
				AddGameObjectPopup(GameObject());
				ImGui::EndMenu();
			}

			ImGui::PopStyleVar();

			ImGui::EndMenuBar();
		}

		ImGui::Separator();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 6.0f));

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(25.0f, 25.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 6.0f));

		if (ImGui::BeginChild(
			"##SceneHierarchyTree",
			ImVec2(0.0f, 0.0f),
			false))
		{
			if (ImGui::BeginPopupContextWindow(
				"ScenePanelContextMenu",
				ImGuiPopupFlags_MouseButtonRight |
				ImGuiPopupFlags_NoOpenOverItems))
			{
				AddGameObjectPopup(GameObject());
				ImGui::EndPopup();
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload =
					ImGui::AcceptDragDropPayload(GameObjectPayloadName))
				{
					if (payload->DataSize == sizeof(UUID))
					{
						UUID droppedUuid =
							*static_cast<UUID*>(payload->Data);

						if (droppedUuid.IsValid())
						{
							GameObject droppedObject =
								m_pSceneContext->Get()->GetGameObject(droppedUuid);

							if (droppedObject.IsValid())
								droppedObject.AttachTo(GameObject(), true);
						}
					}
				}

				ImGui::EndDragDropTarget();
			}

			ImGui::PushStyleColor(ImGuiCol_Header, header);
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, headerHovered);
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, headerActive);

			ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 12.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));

			m_pSceneContext->Get()->ForeachGameObject(
				[this](GameObject obj)
				{
					if (obj.IsRoot())
						DrawGameObjectNode(obj);
				});

			ImGui::PopStyleVar(2);
			ImGui::PopStyleColor(3);
		}

		ImGui::EndChild();

		ImGui::PopStyleVar(3);
	}

	ImGui::EndChild();

	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor(2);

	ImGui::PopStyleVar(2);
}

void ScenePanel::DrawGameObjectNode(GameObject gameObject)
{
	if (!gameObject.IsValid())
		return;

	if (!gameObject.HasComponent<NameComponent>())
		gameObject.AddComponent<NameComponent>();

	const UUID uuid = gameObject.GetUUID();

	auto& nameComponent = gameObject.GetComponent<NameComponent>();
	const std::string& gameObjectName = nameComponent.Name.empty()
		? std::string("Unnamed GameObject")
		: nameComponent.Name;

	SceneComponent& sceneComponent = gameObject.GetComponent<SceneComponent>();
	const bool hasChildren = sceneComponent.begin() != sceneComponent.end();

	ImGuiTreeNodeFlags flags =
		ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_SpanFullWidth;

	if (!hasChildren)
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	if (m_pSelectionContext->Get() == gameObject)
		flags |= ImGuiTreeNodeFlags_Selected;

	ImGui::PushID(static_cast<int>(static_cast<uint64_t>(uuid)));

	const char* icon = GetGameObjectIcon(gameObject);

	ImGui::AlignTextToFramePadding();

	const bool opened = ImGui::TreeNodeEx(
		reinterpret_cast<void*>(static_cast<uint64_t>(uuid)),
		flags,
		"%s %s",
		icon,
		gameObjectName.c_str());

	if (ImGui::IsItemClicked())
		m_pSelectionContext->Set(gameObject);

	bool entityDeleted = false;

	if (ImGui::BeginPopupContextItem())
	{
		ImGui::TextDisabled("%s", gameObjectName.c_str());
		ImGui::Separator();

		AddGameObjectPopup(gameObject);

		ImGui::Separator();

		if (ImGui::MenuItem("Delete"))
			entityDeleted = true;

		ImGui::EndPopup();
	}

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
	{
		m_DraggedGameObject = uuid;

		ImGui::SetDragDropPayload(
			GameObjectPayloadName,
			&m_DraggedGameObject,
			sizeof(UUID));

		ImGui::Text("%s  %s", icon, gameObjectName.c_str());

		ImGui::EndDragDropSource();
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(GameObjectPayloadName))
		{
			if (payload->DataSize == sizeof(UUID))
			{
				UUID droppedUuid = *static_cast<UUID*>(payload->Data);

				if (droppedUuid.IsValid())
				{
					GameObject droppedObject = m_pSceneContext->Get()->GetGameObject(droppedUuid);

					if (droppedObject.IsValid())
					{
						if (droppedObject == gameObject || droppedObject.GetParent() == gameObject)
						{
							droppedObject.DetachFromParent();
						}
						else if (gameObject.GetParent() != droppedObject)
						{
							droppedObject.AttachTo(gameObject);
						}
					}
				}
			}
		}

		ImGui::EndDragDropTarget();
	}

	if (hasChildren && opened)
	{
		for (const auto& child : sceneComponent)
		{
			if (child.IsValid())
				DrawGameObjectNode(child);
		}

		ImGui::TreePop();
	}

	if (entityDeleted)
	{
		if (m_pSelectionContext->Get() == gameObject)
			m_pSelectionContext->Set(GameObject());

		gameObject.Destroy();
	}

	ImGui::PopID();
}

GameObject ScenePanel::AddGameObjectPopup(GameObject parent)
{
	GameObject instance{};

	if (ImGui::BeginMenu("Create"))
	{
		if (ImGui::MenuItem("Empty GameObject"))
		{
			instance = m_pSceneContext->Get()->Instantiate();
			instance.GetComponent<NameComponent>().Name = "Empty GameObject";
		}

		if (ImGui::BeginMenu("Renderer"))
		{
			if (ImGui::MenuItem("Sprite"))
			{
				instance = m_pSceneContext->Get()->Instantiate();
				instance.AddComponent<SpriteRendererComponent>();
				instance.GetComponent<NameComponent>().Name = "Sprite";
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Camera"))
		{
			if (ImGui::MenuItem("Orthographic"))
			{
				instance = m_pSceneContext->Get()->Instantiate();
				instance.AddComponent<CameraComponent>(Camera(1.0f, 1.2f, 0.1f, 1.0f));
				instance.GetComponent<NameComponent>().Name = "Orthographic Camera";
			}

			if (ImGui::MenuItem("Perspective"))
			{
				instance = m_pSceneContext->Get()->Instantiate();
				instance.AddComponent<CameraComponent>(Camera(90.0f, 0.1f, 10.0f));
				instance.GetComponent<NameComponent>().Name = "Perspective Camera";
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenu();
	}

	if (instance.IsValid())
	{
		if (parent.IsValid())
			instance.AttachTo(parent);

		m_pSelectionContext->Set(instance);
	}

	return instance;
}