#include "Panels/MaterialEditorPanel.h"

#include <imgui.h>

#include "Assets/AssetDatabase.h"
#include "Assets/Importer/AssetImporterRegistry.h"

#include <Core/ServiceLocator.h>

#include <UI/UI.h>
#include <UI/IconsFontAwesome7.h>

#include <Asset/ShaderAsset.h>
#include <Asset/TextureAsset.h>

#include <Renderer/Material.h>
#include <Renderer/QuadMaterialData.h>

#include <algorithm>
#include <cstring>

namespace
{
	void SectionHeader(const char* title)
	{
		ImGui::Spacing();
		ImGui::TextDisabled("%s", title);
		ImGui::Separator();
		ImGui::Spacing();
	}

	void BeginPanel(const char* id, ImVec2 size = ImVec2(0.0f, 0.0f))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, ImGui::GetStyle().WindowRounding);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
		ImGui::BeginChild(id, size, true);
	}

	void EndPanel()
	{
		ImGui::EndChild();
		ImGui::PopStyleVar(2);
	}

	bool ToolbarButton(const char* label, ImVec2 size = ImVec2(92.0f, 28.0f))
	{
		return ImGui::Button(label, size);
	}

	const char* PrimitiveToString(Boon::PrimitiveType value)
	{
		switch (value)
		{
		case Boon::PrimitiveType::Lines: return "Lines";
		case Boon::PrimitiveType::Triangles:
		default: return "Triangles";
		}
	}

	const char* BlendToString(Boon::BlendMode value)
	{
		switch (value)
		{
		case Boon::BlendMode::None: return "None";
		case Boon::BlendMode::Additive: return "Additive";
		case Boon::BlendMode::Alpha:
		default: return "Alpha";
		}
	}

	const char* DepthToString(Boon::DepthMode value)
	{
		switch (value)
		{
		case Boon::DepthMode::Disabled: return "Disabled";
		case Boon::DepthMode::Read: return "Read";
		case Boon::DepthMode::ReadWrite:
		default: return "ReadWrite";
		}
	}

	const char* CullToString(Boon::CullMode value)
	{
		switch (value)
		{
		case Boon::CullMode::Back: return "Back";
		case Boon::CullMode::Front: return "Front";
		case Boon::CullMode::None:
		default: return "None";
		}
	}

	void TextMuted(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		ImGui::TextColoredV(ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled), fmt, args);
		va_end(args);
	}
}

namespace BoonEditor
{
	MaterialEditorPanel::MaterialEditorPanel(EditorContext* pContext, const std::string& name)
		: AssetEditor(pContext, name)
	{
	}

	void MaterialEditorPanel::Update()
	{
	}

	void MaterialEditorPanel::RenderToolbar()
	{
		if (!m_Asset.IsValid())
			return;

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Material Editor");

		ImGui::SameLine();
		TextMuted("| Shader, pipeline state, textures and material data");

		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 110.0f);

		if (ToolbarButton(ICON_FA_FLOPPY_DISK " Save"))
		{
			AssetDatabase::Get().Export<MaterialAsset>(m_Asset);

			const auto& path = AssetDatabase::Get().GetPath(m_Asset);
			if (!path.empty())
				ServiceLocator::Get<AssetImporterRegistry>().Import(path);
		}
	}

	void MaterialEditorPanel::RenderMainArea()
	{
		if (!m_Asset.IsValid())
			return;

		MaterialAsset* material = m_Asset.Get();
		if (!material)
			return;

		const ImVec2 avail = ImGui::GetContentRegionAvail();

		const float gap = 8.0f;
		const float leftWidth = std::max(340.0f, avail.x * 0.42f);
		const float rightWidth = std::max(280.0f, avail.x - leftWidth - gap);

		BeginPanel("##material_left_panel", ImVec2(leftWidth, 0.0f));
		RenderMaterialPreview(*material);
		RenderPipelineSettings(*material);
		EndPanel();

		ImGui::SameLine(0.0f, gap);

		BeginPanel("##material_right_panel", ImVec2(rightWidth, 0.0f));
		RenderMaterialData(*material);
		RenderTextureBindings(*material);
		EndPanel();
	}

	void MaterialEditorPanel::RenderPipelineSettings(MaterialAsset& material)
	{
		SectionHeader("Pipeline");

		AssetHandle shader = material.GetShader();
		if (UI::AssetRef("Shader", shader, AssetType::Shader))
			material.SetShader(shader);

		int primitive = static_cast<int>(material.GetPrimitiveType());
		const char* primitiveItems[] = { "Triangles", "Lines" };

		if (UI::Combo("Primitive", primitive, primitiveItems, 2))
			material.SetPrimitiveType(primitive == 1 ? PrimitiveType::Lines : PrimitiveType::Triangles);

		int blend = static_cast<int>(material.GetBlendMode());
		const char* blendItems[] = { "None", "Alpha", "Additive" };

		if (UI::Combo("Blend", blend, blendItems, 3))
		{
			switch (blend)
			{
			case 0: material.SetBlendMode(BlendMode::None); break;
			case 2: material.SetBlendMode(BlendMode::Additive); break;
			case 1:
			default: material.SetBlendMode(BlendMode::Alpha); break;
			}
		}

		int depth = static_cast<int>(material.GetDepthMode());
		const char* depthItems[] = { "Disabled", "Read", "ReadWrite" };

		if (UI::Combo("Depth", depth, depthItems, 3))
		{
			switch (depth)
			{
			case 0: material.SetDepthMode(DepthMode::Disabled); break;
			case 1: material.SetDepthMode(DepthMode::Read); break;
			case 2:
			default: material.SetDepthMode(DepthMode::ReadWrite); break;
			}
		}

		int cull = static_cast<int>(material.GetCullMode());
		const char* cullItems[] = { "None", "Back", "Front" };

		if (UI::Combo("Cull", cull, cullItems, 3))
		{
			switch (cull)
			{
			case 1: material.SetCullMode(CullMode::Back); break;
			case 2: material.SetCullMode(CullMode::Front); break;
			case 0:
			default: material.SetCullMode(CullMode::None); break;
			}
		}

		ImGui::Spacing();
		TextMuted("Current: %s / %s / %s / %s",
			PrimitiveToString(material.GetPrimitiveType()),
			BlendToString(material.GetBlendMode()),
			DepthToString(material.GetDepthMode()),
			CullToString(material.GetCullMode()));
	}

	void MaterialEditorPanel::RenderMaterialData(MaterialAsset& material)
	{
		SectionHeader("Material Data");

		Buffer data = material.GetData();

		if (data.Empty())
		{
			TextMuted("No material data stored.");

			if (ImGui::Button(ICON_FA_PLUS " Create Quad Data", ImVec2(-1.0f, 28.0f)))
			{
				QuadMaterialData quadData{};
				quadData.Color = glm::vec4(1.0f);
				quadData.TilingFactor = 1.0f;

				Buffer buffer(sizeof(QuadMaterialData));
				std::memcpy(buffer.Data(), &quadData, sizeof(QuadMaterialData));

				material.SetData(buffer);
			}

			return;
		}

		if (data.Size() >= sizeof(QuadMaterialData))
		{
			QuadMaterialData quadData{};
			std::memcpy(&quadData, data.Data(), sizeof(QuadMaterialData));

			bool changed = false;

			changed |= UI::ColorPicker("Color", quadData.Color).Changed;
			changed |= UI::DragFloat("Tiling", quadData.TilingFactor, 0.0f, 100.0f, 0.01f).Changed;

			if (changed)
			{
				std::memcpy(data.Data(), &quadData, sizeof(QuadMaterialData));
				material.SetData(data);
			}

			return;
		}

		TextMuted("Raw data size: %llu bytes", static_cast<unsigned long long>(data.Size()));

		if (ImGui::Button(ICON_FA_TRASH " Clear Data", ImVec2(-1.0f, 28.0f)))
			material.SetData(Buffer{});
	}

	void MaterialEditorPanel::RenderTextureBindings(MaterialAsset& material)
	{
		SectionHeader("Textures");

		const auto& textures = material.GetTextures();

		if (textures.empty())
		{
			TextMuted("No texture bindings.");
			ImGui::Spacing();
		}

		for (int i = 0; i < static_cast<int>(textures.size()); ++i)
		{
			const auto& binding = textures[i];

			ImGui::PushID(i);

			BeginPanel("##texture_binding", ImVec2(0.0f, 92.0f));

			ImGui::Text("Binding %d", i);
			TextMuted("Name: %s | Slot: %u", binding.Name.c_str(), binding.Slot);

			AssetHandle textureHandle = binding.TextureHandle;

			if (UI::AssetRef("Texture", textureHandle, AssetType::Texture))
			{
				material.SetTextureBinding(
					static_cast<size_t>(i),
					binding.Name,
					textureHandle,
					binding.Slot);
			}

			EndPanel();

			ImGui::PopID();
			ImGui::Spacing();
		}

		if (ImGui::Button(ICON_FA_PLUS " Add Texture", ImVec2(-1.0f, 28.0f)))
			material.AddTexture("u_Texture", AssetHandle{}, 0);
	}

	void MaterialEditorPanel::RenderMaterialPreview(MaterialAsset& material)
	{
		SectionHeader("Preview");

		const float previewHeight = 240.0f;

		ImVec2 min = ImGui::GetCursorScreenPos();
		ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, previewHeight);
		ImVec2 max = ImVec2(min.x + size.x, min.y + size.y);

		ImGui::InvisibleButton("##material_preview_area", size);

		ImDrawList* drawList = ImGui::GetWindowDrawList();

		drawList->AddRectFilled(min, max, ImGui::GetColorU32(ImGuiCol_FrameBg), 8.0f);
		drawList->AddRect(min, max, ImGui::GetColorU32(ImGuiCol_Border), 8.0f);

		glm::vec4 color{ 1.0f };
		float tiling = 1.0f;

		const Buffer& data = material.GetData();

		if (data.Size() >= sizeof(QuadMaterialData))
		{
			QuadMaterialData quadData{};
			std::memcpy(&quadData, data.Data(), sizeof(QuadMaterialData));

			color = quadData.Color;
			tiling = quadData.TilingFactor;
		}

		ImVec2 center = ImVec2(min.x + size.x * 0.5f, min.y + size.y * 0.55f);
		float quadSize = std::min(size.x, size.y) * 0.35f;

		ImVec2 quadMin = ImVec2(center.x - quadSize, center.y - quadSize);
		ImVec2 quadMax = ImVec2(center.x + quadSize, center.y + quadSize);

		ImU32 tint = ImGui::ColorConvertFloat4ToU32(
			ImVec4(color.r, color.g, color.b, color.a));

		std::shared_ptr<Texture2D> texture = nullptr;

		for (const auto& binding : material.GetTextures())
		{
			if (binding.TextureHandle)
			{
				AssetRef<Texture2DAsset> texRef(binding.TextureHandle);
				if (texRef.IsValid())
				{
					texture = texRef.Instance();
					break;
				}
			}
		}

		if (texture)
		{
			ImTextureID textureId = texture->GetRendererID();

			drawList->AddImage(
				textureId,
				quadMin,
				quadMax,
				ImVec2(0.0f, tiling),
				ImVec2(tiling, 0.0f),
				tint);
		}
		else
		{
			drawList->AddRectFilled(quadMin, quadMax, tint, 8.0f);
		}

		drawList->AddRect(quadMin, quadMax, ImGui::GetColorU32(ImGuiCol_Border), 8.0f, 0, 2.0f);

		drawList->AddText(
			ImVec2(min.x + 12.0f, min.y + 12.0f),
			ImGui::GetColorU32(ImGuiCol_TextDisabled),
			"Material Preview");

		if (!texture)
		{
			const char* label = "No texture";
			ImVec2 textSize = ImGui::CalcTextSize(label);

			drawList->AddText(
				ImVec2(center.x - textSize.x * 0.5f, quadMax.y + 12.0f),
				ImGui::GetColorU32(ImGuiCol_TextDisabled),
				label);
		}
	}
}