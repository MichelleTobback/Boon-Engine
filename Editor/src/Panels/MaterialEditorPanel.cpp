#include "Panels/MaterialEditorPanel.h"

#include <imgui.h>

#include "Assets/AssetDatabase.h"

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
	}

	void BeginEditorPanel(const char* id, ImVec2 size = ImVec2(0.0f, 0.0f))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, ImGui::GetStyle().WindowRounding);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
		ImGui::BeginChild(id, size, true);
	}

	void EndEditorPanel()
	{
		ImGui::EndChild();
		ImGui::PopStyleVar(2);
	}

	bool EditorButton(const char* label, bool active = false, ImVec2 size = ImVec2(78.0f, 26.0f))
	{
		ImGui::PushStyleColor(ImGuiCol_Button, active ? ImGui::GetColorU32(ImGuiCol_FrameBgActive) : ImGui::GetColorU32(ImGuiCol_Button));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetColorU32(ImGuiCol_ButtonHovered));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetColorU32(ImGuiCol_ButtonActive));

		bool pressed = ImGui::Button(label, size);

		ImGui::PopStyleColor(3);
		return pressed;
	}

	const char* PrimitiveToString(PrimitiveType value)
	{
		switch (value)
		{
		case PrimitiveType::Lines: return "Lines";
		case PrimitiveType::Triangles:
		default: return "Triangles";
		}
	}

	const char* BlendToString(BlendMode value)
	{
		switch (value)
		{
		case BlendMode::None: return "None";
		case BlendMode::Additive: return "Additive";
		case BlendMode::Alpha:
		default: return "Alpha";
		}
	}

	const char* DepthToString(DepthMode value)
	{
		switch (value)
		{
		case DepthMode::Disabled: return "Disabled";
		case DepthMode::Read: return "Read";
		case DepthMode::ReadWrite:
		default: return "ReadWrite";
		}
	}

	const char* CullToString(CullMode value)
	{
		switch (value)
		{
		case CullMode::Back: return "Back";
		case CullMode::Front: return "Front";
		case CullMode::None:
		default: return "None";
		}
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
		ImGui::TextDisabled("| Shader, pipeline state, textures, data");

		ImGui::Spacing();

		if (EditorButton(ICON_FA_FLOPPY_DISK " Save", false, ImVec2(92.0f, 28.0f)))
			AssetDatabase::Get().Export<MaterialAsset>(m_Asset);
	}

	void MaterialEditorPanel::RenderMainArea()
	{
		if (!m_Asset.IsValid())
			return;

		MaterialAsset* material = m_Asset.Get();
		if (!material)
			return;

		ImVec2 avail = ImGui::GetContentRegionAvail();

		const float gap = 8.0f;
		const float leftWidth = std::max(320.0f, avail.x * 0.45f);
		const float rightWidth = avail.x - leftWidth - gap;

		BeginEditorPanel("##material_pipeline_panel", ImVec2(leftWidth, 0.0f));
		RenderPipelineSettings(*material);
		EndEditorPanel();

		ImGui::SameLine(0.0f, gap);

		BeginEditorPanel("##material_data_panel", ImVec2(rightWidth, 0.0f));

		BeginEditorPanel("##material_preview_panel", ImVec2(0.0f, 220.0f));
		RenderMaterialPreview(*material);
		EndEditorPanel();

		RenderMaterialData(*material);
		RenderTextureBindings(*material);

		EndEditorPanel();
	}

	void MaterialEditorPanel::RenderPipelineSettings(MaterialAsset& material)
	{
		SectionHeader("Pipeline");

		AssetHandle shader = material.GetShader();

		if (UI::AssetRef("Shader", shader, AssetType::Shader))
			material.SetShader(shader);

		ImGui::Spacing();

		int primitive = static_cast<int>(material.GetPrimitiveType());
		const char* primitiveItems[] = { "Triangles", "Lines" };

		if (ImGui::Combo("Primitive", &primitive, primitiveItems, 2))
		{
			material.SetPrimitiveType(
				primitive == 1 ? PrimitiveType::Lines : PrimitiveType::Triangles);
		}

		int blend = static_cast<int>(material.GetBlendMode());
		const char* blendItems[] = { "None", "Alpha", "Additive" };

		if (ImGui::Combo("Blend", &blend, blendItems, 3))
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

		if (ImGui::Combo("Depth", &depth, depthItems, 3))
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

		if (ImGui::Combo("Cull", &cull, cullItems, 3))
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
		ImGui::TextDisabled("Current");
		ImGui::Text("Primitive: %s", PrimitiveToString(material.GetPrimitiveType()));
		ImGui::Text("Blend: %s", BlendToString(material.GetBlendMode()));
		ImGui::Text("Depth: %s", DepthToString(material.GetDepthMode()));
		ImGui::Text("Cull: %s", CullToString(material.GetCullMode()));
	}

	void MaterialEditorPanel::RenderMaterialData(MaterialAsset& material)
	{
		SectionHeader("Material Data");

		Buffer data = material.GetData();

		if (data.Empty())
		{
			if (EditorButton("Create Quad Data", false, ImVec2(140.0f, 28.0f)))
			{
				QuadMaterialData quadData{};
				quadData.Color = glm::vec4(1.0f);
				quadData.TilingFactor = 1.0f;

				Buffer buffer(sizeof(QuadMaterialData));
				std::memcpy(buffer.Data(), &quadData, sizeof(QuadMaterialData));

				material.SetData(buffer);
			}

			ImGui::TextDisabled("No material data stored.");
			return;
		}

		if (data.Size() >= sizeof(QuadMaterialData))
		{
			QuadMaterialData quadData{};
			std::memcpy(&quadData, data.Data(), sizeof(QuadMaterialData));

			bool changed = false;

			changed |= ImGui::ColorEdit4("Color", &quadData.Color.x);
			changed |= ImGui::DragFloat("Tiling", &quadData.TilingFactor, 0.01f, 0.0f, 100.0f);

			if (changed)
			{
				std::memcpy(data.Data(), &quadData, sizeof(QuadMaterialData));
				material.SetData(data);
			}
		}
		else
		{
			ImGui::TextDisabled("Raw data size: %llu bytes", static_cast<unsigned long long>(data.Size()));
		}
	}

	void MaterialEditorPanel::RenderTextureBindings(MaterialAsset& material)
	{
		SectionHeader("Textures");

		const auto& textures = material.GetTextures();

		if (textures.empty())
			ImGui::TextDisabled("No texture bindings.");

		for (int i = 0; i < static_cast<int>(textures.size()); ++i)
		{
			const auto& binding = textures[i];

			ImGui::PushID(i);

			ImGui::Text("Binding %d", i);
			ImGui::TextDisabled("Name: %s | Slot: %u", binding.Name.c_str(), binding.Slot);

			AssetHandle textureHandle = binding.TextureHandle;

			// This requires a SetTextureBinding/ReplaceTextureBinding function on MaterialAsset.
			if (UI::AssetRef("Texture", textureHandle, AssetType::Texture))
			{
				material.SetTextureBinding(
					i,
					binding.Name,
					textureHandle,
					binding.Slot);
			}

			ImGui::Separator();
			ImGui::PopID();
		}

		ImGui::Spacing();

		if (EditorButton("Add Texture", false, ImVec2(112.0f, 28.0f)))
		{
			material.AddTexture("u_Texture", AssetHandle{}, 0);
		}
	}

	void BoonEditor::MaterialEditorPanel::RenderMaterialPreview(MaterialAsset& material)
	{
		ImGui::BeginChild(
			"##material_preview_viewport",
			ImGui::GetContentRegionAvail(),
			false,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
		);

		ImDrawList* drawList = ImGui::GetWindowDrawList();

		ImVec2 min = ImGui::GetCursorScreenPos();
		ImVec2 size = ImGui::GetContentRegionAvail();
		ImVec2 max(min.x + size.x, min.y + size.y);

		drawList->AddRectFilled(min, max, ImGui::GetColorU32(ImGuiCol_FrameBg), 8.0f);

		ImVec2 center(min.x + size.x * 0.5f, min.y + size.y * 0.5f);
		float previewSize = std::min(size.x, size.y) * 0.45f;

		ImVec2 quadMin(center.x - previewSize, center.y - previewSize);
		ImVec2 quadMax(center.x + previewSize, center.y + previewSize);

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

		ImU32 tint = ImGui::ColorConvertFloat4ToU32(
			ImVec4(color.r, color.g, color.b, color.a)
		);

		std::shared_ptr<Texture2D> texture = nullptr;

		for (const auto& binding : material.GetTextures())
		{
			if (binding.Name == "u_Texture" && binding.TextureHandle)
			{
				AssetRef<Texture2DAsset> texRef(binding.TextureHandle);

				if (texRef.IsValid())
					texture = texRef.Instance();

				break;
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
				tint
			);
		}
		else
		{
			drawList->AddRectFilled(quadMin, quadMax, tint, 8.0f);
		}

		drawList->AddRect(quadMin, quadMax, ImGui::GetColorU32(ImGuiCol_Border), 8.0f, 0, 2.0f);

		drawList->AddText(
			ImVec2(min.x + 12.0f, min.y + 12.0f),
			ImGui::GetColorU32(ImGuiCol_TextDisabled),
			"Material Preview"
		);

		ImGui::Dummy(size);
		ImGui::EndChild();
	}
}