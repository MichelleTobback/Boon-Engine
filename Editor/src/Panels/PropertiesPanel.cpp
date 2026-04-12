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
#include <Core/EditorContext.h>
#include <Command/EditorCommandQueue.h>
#include <Command/PropertyCommand.h>

using namespace BoonEditor;

BoonEditor::PropertiesPanel::PropertiesPanel(const std::string& name, EditorContext* pContext, GameObjectContext* pGameObject)
	: EditorPanel(name, pContext), m_pGameObjectContext{pGameObject}
{
	
}

void BoonEditor::PropertiesPanel::OnRenderUI()
{
	if (!m_pGameObjectContext || !m_pGameObjectContext->IsValid() || !m_pGameObjectContext->Get().IsValid())
		return;

    if (!m_pGameObjectContext->Get().HasComponent<NameComponent>())
        m_pGameObjectContext->Get().AddComponent<NameComponent>();

    NameComponent& nameComponent{ m_pGameObjectContext->Get().GetComponent<NameComponent>() };
    std::string gameObjectName{ nameComponent.Name };
    const size_t nameBufferSize{ 64 };
    char nameBuffer[nameBufferSize];
    memset(nameBuffer, 0, sizeof(nameBuffer));
    strcpy_s(nameBuffer, sizeof(nameBuffer), gameObjectName.c_str());
    if (ImGui::InputText("##gameObjectName", nameBuffer, sizeof(nameBuffer)))
        nameComponent.Name = std::string(nameBuffer);

    RenderComponentNode<TransformComponent>("Transform", [this](TransformComponent& transform)
        {
            BClassRegistry& reg = BClassRegistry::Get();
            BClass* cls = reg.Find<TransformComponent>();
            const BProperty* pPosProp = cls->FindProperty("m_LocalPosition");
            void* pInstance = static_cast<void*>(&transform);
            glm::vec3 value = transform.GetLocalPosition();
            UI::PropertyResult result = UI::Float3Control("Position", value);
            if (result)
            {
                if (result.Committed)
                    GetContext().GetCommandQueue()->Push<SetComponentPropertyCommand>(
                        m_pGameObjectContext->Get(), cls, pPosProp, result.OldValue, value);
                transform.SetLocalPosition(value);
            }
            value = transform.GetLocalEulerRotation();
            const BProperty* pRotProp = cls->FindProperty("m_LocalEuler");
            result = UI::Float3Control("rotation", value);
            if (result)
            {
                if (result.Committed)
                    GetContext().GetCommandQueue()->Push<SetComponentPropertyCommand>(
                        m_pGameObjectContext->Get(), cls, pRotProp, result.OldValue, value);
                transform.SetLocalRotation(value);
            }
            value = transform.GetLocalScale();
            const BProperty* pScaleProp = cls->FindProperty("m_LocalScale");
            result = UI::Float3Control("scale", value);
            if (result)
            {
                if (result.Committed)
                    GetContext().GetCommandQueue()->Push<SetComponentPropertyCommand>(
                        m_pGameObjectContext->Get(), cls, pScaleProp, result.OldValue, value);
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
            if (m_pGameObjectContext->Get().HasComponentByClass(&cls))
                RenderComponentNode(&cls);
        });

    // Detect right click on the window
    if (ImGui::BeginPopupContextWindow("add component"))
    {
        
        BClassRegistry::Get().ForEach([this](const BClass& cls)
            {
                if (m_pGameObjectContext->Get().HasComponentByClass(&cls))
                    return;

                if (ImGui::MenuItem(cls.name.c_str()))
                {
                    m_pGameObjectContext->Get().AddComponentFromClass(&cls);
                }
            });

        ImGui::EndPopup();
    }
}

inline void PropertiesPanel::RenderComponentNode(BClass* cls, const std::function<void()>& fn)
{
    GameObject& owner = m_pGameObjectContext->Get();
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
        cls->ForEachProperty([pInstance, this, owner, cls](const BProperty& prop)
            {
                if (prop.IsVariant())
                {
                    UI::PropertyResult result = UI::Property(prop, pInstance);
                    if (result.Committed)
                    {
                        Variant newVal = cls->GetValue(pInstance, prop.name.c_str());
                        GetContext().GetCommandQueue()->Push<SetComponentPropertyCommand>(
                            owner, cls, &prop, result.OldValue, newVal);
                    }
                }
                else
                {
                    UI::Property(prop, pInstance);
                }
            });

        if (fn) fn();
        ImGui::TreePop();
    }

    if (removeComponent)
        owner.RemoveComponentByClass(cls);
}
