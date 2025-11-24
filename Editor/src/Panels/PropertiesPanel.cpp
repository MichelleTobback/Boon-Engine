#include "Panels/PropertiesPanel.h"
#include "UI/UI.h"

#include <Component/CameraComponent.h>
#include <Component/NameComponent.h>
#include <Component/SpriteRendererComponent.h>
#include <Component/SpriteAnimatorComponent.h>
#include <Component/BoxCollider2D.h>

#include <Asset/AssetLibrary.h>
#include <Asset/SpriteAtlasAsset.h>

#include "Renderer/Texture.h"

#include <Core/ServiceLocator.h>

using namespace BoonEditor;

BoonEditor::PropertiesPanel::PropertiesPanel(const std::string& name, DragDropRouter* pRouter, GameObjectContext* pContext)
	: EditorPanel(name, pRouter), m_pContext{pContext}
{
	
}

void BoonEditor::PropertiesPanel::OnRenderUI()
{
	if (!m_pContext || !m_pContext->Get().IsValid())
		return;

    if (!m_pContext->Get().HasComponent<NameComponent>())
        m_pContext->Get().AddComponent<NameComponent>();

    NameComponent& nameComponent{ m_pContext->Get().GetComponent<NameComponent>() };
    std::string gameObjectName{ nameComponent.Name };
    const size_t nameBufferSize{ 64 };
    char nameBuffer[nameBufferSize];
    memset(nameBuffer, 0, sizeof(nameBuffer));
    strcpy_s(nameBuffer, sizeof(nameBuffer), gameObjectName.c_str());
    if (ImGui::InputText("##gameObjectName", nameBuffer, sizeof(nameBuffer)))
        nameComponent.Name = std::string(nameBuffer);

    RenderComponentNode<TransformComponent>("Transform", [this](TransformComponent& transform)
        {
            glm::vec3 value = transform.GetLocalPosition();
            if (RenderFloat3Control("Position", value))
            {
                transform.SetLocalPosition(value);
            }
            value = transform.GetLocalEulerRotation();
            if (RenderFloat3Control("rotation", value))
            {
                transform.SetLocalRotation(value);
            }
            value = transform.GetLocalScale();
            if (RenderFloat3Control("scale", value))
            {
                transform.SetLocalScale(value);
            }
        });

    RenderComponentNode<CameraComponent>("Camera", [](CameraComponent& cameraComp)
        {
            Camera& camera = cameraComp.Camera;

            int projectionType = (int)camera.GetProjectionType();
            const char* projectionTypes[] = { "Perspective", "Orthographic" };

            if (UI::Combo("Mode", projectionType, projectionTypes, IM_ARRAYSIZE(projectionTypes)))
            {
                camera.SetProjectionType((Camera::ProjectionType)projectionType);
            }

            switch (camera.GetProjectionType())
            {
            case Camera::ProjectionType::Perspective:
            {
                float fov = camera.GetFov();
                if (UI::SliderFloat("FOV", fov, 1.0f, 180.0f))
                {
                    camera.SetFov(fov);
                }
            }
            break;
            case Camera::ProjectionType::Orthographic:
            {
                float size = camera.GetSize();
                if (UI::SliderFloat("size", size, 1.0f, 100.0f))
                {
                    camera.SetSize(size);
                }
            }
            break;
            }

            float value = camera.GetNear();
            if (UI::DragFloat("near", value))
            {
                camera.SetNear(value);
            }
            value = camera.GetFar();
            if (UI::DragFloat("far", value))
            {
                camera.SetFar(value);
            }
        });

    BClassRegistry& reg = BClassRegistry::Get();
    reg.ForEach([this](BClass& cls)
        {
            if (m_pContext->Get().HasComponentByClass(&cls))
                RenderComponentNode(&cls);
        });

    // Detect right click on the window
    if (ImGui::BeginPopupContextWindow("add component"))
    {
        
        BClassRegistry::Get().ForEach([this](const BClass& cls)
            {
                if (m_pContext->Get().HasComponentByClass(&cls))
                    return;

                if (ImGui::MenuItem(cls.name.c_str()))
                {
                    m_pContext->Get().AddComponentFromClass(&cls);
                }
            });

        ImGui::EndPopup();
    }
}

inline void PropertiesPanel::RenderComponentNode(BClass* cls, const std::function<void()>& fn)
{
    GameObject& owner = m_pContext->Get();
    if (!owner.HasComponentByClass(cls))
        return;

    if (cls->HasMeta("HideInInspector"))
        return;

    // Compatibility fallback for older Dear ImGui versions
#ifndef ImGuiTreeNodeFlags_AllowItemOverlap
#define ImGuiTreeNodeFlags_AllowItemOverlap (1 << 20)
#endif

    const ImGuiTreeNodeFlags treeNodeFlags =
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_Framed |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_FramePadding |
        ImGuiTreeNodeFlags_AllowItemOverlap;

    ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });

    // Use version-safe way to get line height
    float lineHeight = ImGui::GetTextLineHeight();

    ImGui::Separator();

    // Use the owner UUID correctly
    std::string id = cls->name + std::to_string(owner.GetUUID());
    std::string name = cls->HasMeta("Name") ? cls->GetMeta("Name").value() : cls->name;
    bool open = ImGui::TreeNodeEx(id.c_str(), treeNodeFlags, "%s", name.c_str());

    ImGui::PopStyleVar();

    // Position the "+" button on the far right
    ImGui::SameLine(contentRegionAvailable.x - lineHeight);
    if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
    {
        ImGui::OpenPopup("ComponentSettings");
    }

    bool removeComponent = false;
    if (ImGui::BeginPopup("ComponentSettings"))
    {
        if (ImGui::MenuItem("Remove component"))
            removeComponent = true;

        ImGui::EndPopup();
    }

    if (open)
    {
        void* pInstance = owner.GetComponentByClass(cls);
        cls->ForEachProperty([pInstance](const BProperty& prop)
            {
                UI::Property(prop, pInstance);
            });

        if (fn) fn();
        ImGui::TreePop();
    }

    if (removeComponent)
        owner.RemoveComponentByClass(cls);
}
