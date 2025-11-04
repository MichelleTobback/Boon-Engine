#include "Panels/PropertiesPanel.h"
#include "UI/UI.h"

#include <Component/CameraComponent.h>
#include <Component/NameComponent.h>
#include <Component/SpriteRendererComponent.h>
#include <Component/SpriteAnimatorComponent.h>

#include <Asset/AssetLibrary.h>
#include <Asset/SpriteAtlasAsset.h>

#include "Renderer/Texture.h"

#include <Core/ServiceLocator.h>

using namespace BoonEditor;

BoonEditor::PropertiesPanel::PropertiesPanel(GameObjectContext* pContext)
	: EditorPanel("Properties"), m_pContext{pContext}
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

    RenderComponentNode<SpriteRendererComponent>("Sprite renderer", [this](SpriteRendererComponent& spriteComponent)
        {
            auto atlas = ServiceLocator::Get<AssetLibrary>().GetAsset<SpriteAtlasAsset>(spriteComponent.SpriteAtlasHandle);

            glm::vec4 value4 = spriteComponent.Color;
            if (UI::ColorPicker("Tint", value4))
            {
                spriteComponent.Color = value4;
            }

            float value = spriteComponent.Tiling;
            if (UI::DragFloat("Tiling", value, 0.1f, 10.f, 0.1f))
            {
                spriteComponent.Tiling = value;
            }

            auto tex = atlas->GetTexture();
            ImGui::ImageButton("Atlas", tex->GetRendererID(), {25.f, 25.f});
        });

    RenderComponentNode<SpriteAnimatorComponent>("Sprite animator", [this](SpriteAnimatorComponent& spriteComponent)
        {
            auto& pAtlas = spriteComponent.Atlas;
            int clip = spriteComponent.Clip;
            if (UI::SliderInt("Clip", clip, 0, pAtlas->GetClips().size() - 1))
            {
                spriteComponent.Clip = clip;
            }

            float value = spriteComponent.GetClip().Speed;
            if (UI::DragFloat("Speed", value, 0.1f, 10.f, 0.1f))
            {
                spriteComponent.GetClip().Speed = value;
            }
        });
}
