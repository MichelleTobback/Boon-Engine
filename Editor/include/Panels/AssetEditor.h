#pragma once

#include <Core/BoonEditor.h>
#include <Core/EditorContext.h>

#include "Panels/EditorPanel.h"
#include "Panels/IViewportCanvasRenderer.h"

#include <Asset/AssetLibrary.h>

#include "UI/UI.h"

#include <glm/glm.hpp>

namespace BoonEditor
{
    class ViewportPanel;

    class AssetEditorBase : public EditorPanel, public IViewportCanvasRenderer
    {
    public:
        AssetEditorBase(EditorContext* pContext, const std::string& name)
            : EditorPanel(pContext, name) {
        }

        virtual void RenderUI() override;

        inline ViewportPanel* GetViewport() const { return m_pViewport; }

    protected:
        virtual bool SetContext(AssetHandle handle) = 0;

        virtual void RenderToolbar() {}
        virtual void RenderMainArea() {}

    private:
        friend class AssetEditorPanel;

        ViewportPanel* m_pViewport = nullptr;
    };

    template<typename TAsset>
    class AssetEditor : public AssetEditorBase
    {
    public:
        AssetEditor(EditorContext* pContext, const std::string& name);

        virtual void Update() override;
        virtual void OnRenderUI() override;

        inline AssetRef<TAsset>& GetAsset() { return m_Asset; }
        inline void SetAsset(AssetRef<TAsset> asset) { m_Asset = asset; }

    protected:
        virtual bool SetContext(AssetHandle handle) override;

        virtual void RenderToolbar() override {}
        virtual void RenderMainArea() override {}

        AssetRef<TAsset> m_Asset{};

    private:
    };

    template<typename TAsset>
    inline AssetEditor<TAsset>::AssetEditor(EditorContext* pContext, const std::string& name)
        : AssetEditorBase(pContext, name) {
    }

    template<typename TAsset>
    inline void AssetEditor<TAsset>::Update()
    {
    }

    template<typename TAsset>
    inline void AssetEditor<TAsset>::OnRenderUI()
    {
        if (!m_Asset.IsValid())
        {
            ImGui::Text("Invalid Asset");
            return;
        }

        RenderToolbar();
        RenderMainArea();
    }

    template<typename TAsset>
    inline bool AssetEditor<TAsset>::SetContext(AssetHandle handle)
    {
        AssetLibrary& lib = *GetContext().GetEngineContext().AssetLib;

        const AssetMeta* pMeta = lib.GetMeta(handle);
        if (!pMeta)
            return false;

        if (pMeta->type != AssetTraits<TAsset>::Type)
            return false;

        m_Asset = AssetRef<TAsset>(handle);
        return true;
    }
}