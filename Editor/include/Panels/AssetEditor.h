#pragma once
#include <Core/BoonEditor.h>
#include "Panels/EditorPanel.h"
#include "Renderer/SceneRenderer.h"
#include "Core/EditorCamera.h"

#include <Core/ServiceLocator.h>
#include <Asset/AssetLibrary.h>
#include <Scene/Scene.h>
#include <Scene/SceneSerializer.h>

#include "UI/UI.h"
#include <vector>
#include <glm/glm.hpp>

namespace BoonEditor
{
    class ViewportPanel;

    class AssetEditorBase : public EditorPanel
    {
    public:
        AssetEditorBase(const std::string& name, DragDropRouter* pRouter)
            : EditorPanel(name, pRouter) { }

        virtual void OnRender() override;

        inline Scene& GetScene() { return *m_PrevScene->Get(); }
        inline ViewportPanel* GetViewport() const { return m_pViewport; }

    protected:
        virtual bool SetContext(AssetHandle handle) = 0;

        virtual void RenderToolbar() {}
        virtual void RenderMainArea() {}

        glm::vec3 ScreenToWorld(const glm::vec2& mousePos);

    private:
        friend class AssetEditorPanel;

        SceneContext* m_PrevScene;
        ViewportPanel* m_pViewport = nullptr;
    };

    template<typename TAsset>
    class AssetEditor : public AssetEditorBase
    {
    public:
        AssetEditor(const std::string& name, DragDropRouter* pRouter);

        virtual void Update() override;
        virtual void OnRenderUI() override;

        inline AssetRef<TAsset>& GetAsset() { return m_Asset; }
        inline void SetAsset(AssetRef<TAsset> asset) { m_Asset = asset; }

    protected:
        virtual bool SetContext(AssetHandle handle) override;

        virtual void RenderToolbar() override {}
        virtual void RenderMainArea() override {}
        virtual void BuildPreviewScene(Scene& scene) {}

        AssetRef<TAsset> m_Asset{};

    private:
    };

    template<typename TAsset>
    inline AssetEditor<TAsset>::AssetEditor(const std::string& name, DragDropRouter* pRouter)
        : AssetEditorBase(name, pRouter){ }

    template<typename TAsset>
    inline void AssetEditor<TAsset>::Update()
    {
    }

    template<typename TAsset>
    inline void AssetEditor<TAsset>::OnRenderUI()
    {
        if (!m_Asset.IsValid()) { ImGui::Text("Invalid Asset"); return; }

        RenderToolbar();
        RenderMainArea();
    }

    template<typename TAsset>
    inline bool AssetEditor<TAsset>::SetContext(AssetHandle handle)
    {
        AssetLibrary& lib = ServiceLocator::Get<AssetLibrary>();
        const AssetMeta* pMeta = lib.GetMeta(handle);
        if (!pMeta)
            return false;

        if (pMeta->type != AssetTraits<TAsset>::Type)
            return false;

        m_Asset = AssetRef<TAsset>(handle);
        SceneSerializer ser(GetScene());
        ser.Clear();
        BuildPreviewScene(GetScene());
        return true;
    }
}
