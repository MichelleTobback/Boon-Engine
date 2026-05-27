#pragma once

#include "Panels/AssetEditor.h"

#include <Asset/MaterialAsset.h>

#include <string>

using namespace Boon;

namespace BoonEditor
{
	class MaterialEditorPanel : public AssetEditor<MaterialAsset>
	{
	public:
		MaterialEditorPanel(EditorContext* pContext, const std::string& name);

		virtual void Update() override;

		virtual bool CanRenderViewport() const override { return false; }

	protected:
		virtual void RenderToolbar() override;
		virtual void RenderMainArea() override;

	private:
		void RenderPipelineSettings(MaterialAsset& material);
		void RenderMaterialData(MaterialAsset& material);
		void RenderTextureBindings(MaterialAsset& material);
		void RenderMaterialPreview(MaterialAsset& material);
	};
}