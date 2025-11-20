#include "Panels/ScenePanel.h"

#include <Component/UUIDComponent.h>
#include <Component/NameComponent.h>
#include <Component/SceneComponent.h>
#include <Component/SpriteRendererComponent.h>
#include <Component/CameraComponent.h>

#include <imgui.h>

using namespace BoonEditor;

BoonEditor::ScenePanel::ScenePanel(const std::string& name, DragDropRouter* pRouter, SceneContext* pScene, GameObjectContext* pSelectedGameObject)
	: EditorPanel(name, pRouter), m_pSceneContext{pScene}, m_pSelectionContext{pSelectedGameObject}
{
}

void BoonEditor::ScenePanel::OnRenderUI()
{
	if (ImGui::BeginPopupContextWindow(0, 1))
	{
		AddGameObjectPopup(GameObject());
		ImGui::EndPopup();
	}

	if (ImGui::BeginDragDropTarget())
	{
		const std::string payloadName{ "GameObject" };
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadName.c_str()))
		{
			if (payload->DataSize > 0)
			{
				UUID& droppedData{ *static_cast<UUID*>(payload->Data) };
				if (droppedData.IsValid())
				{
					GameObject pDroppedObject{ m_pSceneContext->Get()->GetGameObject(droppedData) };
					pDroppedObject.AttachTo(GameObject(), true);
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	m_pSceneContext->Get()->ForeachGameObject([this](GameObject obj)
		{
			if (obj.IsRoot())
				DrawGameObjectNode(obj);
		});
}

void BoonEditor::ScenePanel::DrawGameObjectNode(GameObject gameObject)
{
	UUID uuid{ gameObject.GetUUID() };

	ImGuiTreeNodeFlags flags{ ((m_pSelectionContext->Get() == gameObject) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow};
	flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

	if (!gameObject.HasComponent<NameComponent>())
		gameObject.AddComponent<NameComponent>();
	const std::string& gameObjectName{ gameObject.GetComponent<NameComponent>().Name };
	bool opened{ ImGui::TreeNodeEx((void*)uint64_t(uuid), flags, gameObjectName.c_str()) };

	if (ImGui::IsItemClicked())
	{
		m_pSelectionContext->Set(gameObject);
	}

	//popup
	bool entityDeleted{ false };
	if (ImGui::BeginPopupContextItem())
	{
		AddGameObjectPopup(gameObject);

		ImGui::Separator();

		if (ImGui::MenuItem("Delete entity"))
		{
			entityDeleted = true;
		}
		ImGui::EndPopup();
	}

	//dragdrop
	const std::string payloadName{ "GameObject" };
	if (ImGui::BeginDragDropSource())
	{
		m_DraggedGameObject = gameObject.GetUUID();
		ImGui::SetDragDropPayload(payloadName.c_str(), &m_DraggedGameObject, sizeof(UUID));
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadName.c_str()))
		{
			if (payload->DataSize > 0)
			{
				UUID& droppedData{ *static_cast<UUID*>(payload->Data) };
				if (droppedData.IsValid())
				{
					GameObject droppedObject{ m_pSceneContext->Get()->GetGameObject(droppedData) };
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
		ImGui::EndDragDropTarget();
	}

	if (opened)
	{
		auto sceneComponent{ gameObject.GetComponent<SceneComponent>() };

		for (auto& child : sceneComponent)
		{
			if (child.IsValid())
				DrawGameObjectNode(child);
		}
		ImGui::TreePop();
	}

	if (entityDeleted)
	{
		gameObject.Destroy();
	}
}

GameObject BoonEditor::ScenePanel::AddGameObjectPopup(GameObject parent)
{
	GameObject instance{};
	if (ImGui::BeginMenu("new"))
	{
		if (ImGui::MenuItem("empty"))
		{
			instance = m_pSceneContext->Get()->Instantiate();
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
				instance.AddComponent<CameraComponent>(Camera(1.f, 1.2f, 0.1f, 1.f));
				instance.GetComponent<NameComponent>().Name = "Camera";
			}
			if (ImGui::MenuItem("Perspective"))
			{
				instance = m_pSceneContext->Get()->Instantiate();
				instance.AddComponent<CameraComponent>(Camera(90.f, 0.1f, 10.f));
				instance.GetComponent<NameComponent>().Name = "Camera";
			}
			ImGui::EndMenu();
		}

		ImGui::EndMenu();
	}
	return instance;
}
