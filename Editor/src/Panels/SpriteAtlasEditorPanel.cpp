#include "Panels/SpriteAtlasEditorPanel.h"

#include <imgui.h>
#include <Core/ServiceLocator.h>
#include <Input/Input.h>
#include <Scene/SceneManager.h>
#include <Component/TextureRendererComponent.h>
#include "Panels/ViewportPanel.h"
#include <BoonDebug/DebugRenderer.h>
#include <Renderer/Texture.h>
#include <UI/UI.h>

#include "Assets/AssetDatabase.h"

namespace BoonEditor
{
	SpriteAtlasEditorPanel::SpriteAtlasEditorPanel(const std::string& name, DragDropRouter* pRouter)
		: AssetEditor(name, pRouter){ }

	void SpriteAtlasEditorPanel::BuildPreviewScene(Scene& scene)
	{
		scene.Instantiate().AddComponent<TextureRendererComponent>().Texture = m_Asset->GetInstance()->GetTexture();
	}


	void SpriteAtlasEditorPanel::Update()
	{
		if (!m_Asset.IsValid())
			return;

		if (!m_Asset->GetInstance()->GetTexture().IsValid())
			return;

		Input& input = ServiceLocator::Get<Input>();

		auto& tex = m_Asset->GetInstance()->GetTexture().Instance();
		float ppu = 32.0f;
		glm::vec2 pixelSize = glm::vec2(tex->GetWidth(), tex->GetHeight());
		glm::vec2 worldSize = pixelSize / ppu;

		// ------------------------------------------------------------
		// Mouse inside viewport?
		// ------------------------------------------------------------
		glm::vec2 mp = GetViewport()->GetMousePosition();
		glm::vec2 vp = GetViewport()->GetSize();

		//if (mp.x < 0 || mp.y < 0 || mp.x > vp.x || mp.y > vp.y)
		//	return;

		// ------------------------------------------------------------
		// Convert mouse → world → tile coordinates
		// ------------------------------------------------------------
		glm::vec3 world = ScreenToWorld(mp);
		glm::vec2 mousePos = world;

		std::vector<std::function<void()>> drawqueue;

		switch (m_Mode)
		{
		case AtlasEditorMode::Select:
		{
			for (int i : m_Asset->GetInstance()->GetAllFrameIDs())
			{
				auto& sprite = m_Asset->GetInstance()->GetSpriteFrame(i);
				glm::vec3 worldPos = { (sprite.UV.x - 0.5f) * worldSize.x, (sprite.UV.y - 0.5f) * worldSize.y, 0.f };
				glm::vec2 size = sprite.Size * worldSize;

				bool inside = false;
				if (mousePos.x >= worldPos.x && mousePos.y >= worldPos.y &&
					mousePos.x <= worldPos.x + size.x && mousePos.y <= worldPos.y + size.y)
				{
					inside = true;
					if (input.IsMousePressed(Mouse::ButtonLeft))
						m_SelectedSprite = m_SelectedSprite == i ? -1 : i;
				}

				worldPos.x += (size.x * 0.5f);
				worldPos.y += (size.y * 0.5f);
				size -= 0.005f;
				bool selected = i == m_SelectedSprite;
				glm::vec4 color = inside ? glm::vec4(0.9f, 0.f, 0.99f, 1.f) : selected ? glm::vec4(0.5f, 0.f, 0.9f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 1.f);
				if (inside || selected)
				{
					drawqueue.push_back([worldPos, size, color]()
						{
							DEBUG_DRAW_RECT(worldPos, size, color);
						});
				}
				else
					DEBUG_DRAW_RECT(worldPos, size, color);
			}
			break;
		}
		case AtlasEditorMode::Create:
		{
			if (m_GridTileSize.x == 0) m_GridTileSize.x = 1;
			if (m_GridTileSize.y == 0) m_GridTileSize.y = 1;
			if (m_Cols == 0) m_Cols = 1;
			if (m_Rows == 0) m_Rows = 1;

			auto& tex = m_Asset->GetInstance()->GetTexture().Instance();
			float texW = (float)tex->GetWidth();
			float texH = (float)tex->GetHeight();

			bool useSize = m_GridMode == GridMode::Cellsize;

			// ------------------------------------------------------
			// Number of tiles
			// ------------------------------------------------------
			int xCount = useSize ? (int)(texW / m_GridTileSize.x) : m_Cols;
			int yCount = useSize ? (int)(texH / m_GridTileSize.y) : m_Rows;

			// ------------------------------------------------------
			// Compute world tile size
			// ------------------------------------------------------
			glm::vec2 pixelTileSize = useSize ?
				glm::vec2(m_GridTileSize.x, m_GridTileSize.y) :
				glm::vec2(texW / m_Cols, texH / m_Rows);

			glm::vec2 worldTileSize = pixelTileSize / ppu;

			// ------------------------------------------------------
			// Convert mouse to world
			// ------------------------------------------------------
			glm::vec3 world = ScreenToWorld(mp);
			glm::vec2 mousePos = world;

			// ------------------------------------------------------
			// Iterate tiles
			// ------------------------------------------------------
			int tileIndex = 0;
			for (int ty = 0; ty < yCount; ty++)
			{
				for (int tx = 0; tx < xCount; tx++)
				{
					// ------------------------------------------------------
					// UV for top-left of the tile
					// ------------------------------------------------------
					glm::vec2 uv;
					uv.x = tx * (pixelTileSize.x / texW);
					uv.y = ty * (pixelTileSize.y / texH);

					// ------------------------------------------------------
					// Convert UV into world space (MATCHES SELECT MODE!)
					// ------------------------------------------------------
					glm::vec3 worldPos = {
						(uv.x - 0.5f) * worldSize.x,
						(uv.y - 0.5f) * worldSize.y,
						0.f
					};

					glm::vec2 size = worldTileSize;

					// Tile is drawn centered → shift by half size
					worldPos.x += size.x * 0.5f;
					worldPos.y += size.y * 0.5f;

					// ------------------------------------------------------
					// Hover/select detection
					// ------------------------------------------------------
					bool inside =
						mousePos.x >= worldPos.x - size.x * 0.5f &&
						mousePos.y >= worldPos.y - size.y * 0.5f &&
						mousePos.x <= worldPos.x + size.x * 0.5f &&
						mousePos.y <= worldPos.y + size.y * 0.5f;

					if (inside && input.IsMousePressed(Mouse::ButtonLeft))
					{
						bool exists = m_SelectedTiles.find(tileIndex) != m_SelectedTiles.end();
						if (!exists)
							m_SelectedTiles.insert(tileIndex);
						else
							m_SelectedTiles.erase(tileIndex);
					}

					bool selected = m_SelectedTiles.find(tileIndex) != m_SelectedTiles.end();

					glm::vec4 color =
						inside ? glm::vec4(0.9f, 0.f, 0.99f, 1.f) :
						selected ? glm::vec4(0.5f, 0.f, 0.9f, 1.f) :
						glm::vec4(1.f, 1.f, 1.f, 1.f);

					// ------------------------------------------------------
					// Draw tile outline
					// ------------------------------------------------------
					if (inside || selected)
					{
						drawqueue.push_back([worldPos, size, color]()
							{
								DEBUG_DRAW_RECT(worldPos, size, color);
							});
					}
					else
					{
						DEBUG_DRAW_RECT(worldPos, size, color);
					}

					tileIndex++;
				}
			}

			break;
		}

		}
		for (auto& fn : drawqueue)
		{
			fn();
		}
	}

	void SpriteAtlasEditorPanel::RenderToolbar()
	{
        std::shared_ptr<SpriteAtlas> atlas = m_Asset->GetInstance();
        if (ImGui::Button("save"))
        {
            AssetDatabase::Get().Export<SpriteAtlasAsset>(m_Asset);
        }

        ImGui::SameLine(0.f, 25.f);

		if (ImGui::Button("Select"))  m_Mode = AtlasEditorMode::Select;
		ImGui::SameLine();
		if (ImGui::Button("Create"))  m_Mode = AtlasEditorMode::Create;
	}

    void SpriteAtlasEditorPanel::RenderMainArea()
    {
        std::shared_ptr<SpriteAtlas> atlas = m_Asset->GetInstance();

		AssetHandle tex = atlas->GetTexture();
		if (UI::AssetRef("texture", tex, AssetType::Texture))
		{
			atlas->SetTexture(AssetRef<Texture2DAsset>(tex));
		}

		if (!atlas->GetTexture().IsValid())
			return;

        // Original data (from atlas)
        auto& entries = atlas->GetFrameEntries();

        // -----------------------------------
        // Make a SHADOW COPY for the UI to edit
        // -----------------------------------
        std::vector<FrameEntry> shadow = entries;

        bool uiChanged = UI::List<FrameEntry>("Frames", shadow, m_SelectedSprite,
			[&](const std::string& label, FrameEntry& entry) -> bool
			{
				bool localChanged = false;

				ImGui::Image(atlas->GetTexture()->GetInstance()->GetRendererID(), ImVec2(20.f, 20.f), { entry.frame.UV.x, entry.frame.UV.y }, {entry.frame.Size.x, entry.frame.Size.y});

				ImGui::SameLine();

				ImGui::Text("Stable ID: %d", entry.stableId);

                return localChanged;
            }
        );

        if (uiChanged)
		{
			// -----------------------------------
			// Detect structural changes (add/remove/reorder)
			// -----------------------------------

			// Case A: ADD
			int newId = -1;
			if (shadow.size() > entries.size())
			{
				SpriteFrame blank{};
				newId = atlas->AddSpriteFrame(blank);
			}

			// Case B: REMOVE
			if (shadow.size() < entries.size())
			{
				for (const FrameEntry& oldEntry : entries)
				{
					bool stillExists = false;
					for (const FrameEntry& sh : shadow)
						if (sh.stableId == oldEntry.stableId)
							stillExists = true;

					if (!stillExists)
					{
						atlas->RemoveSpriteFrame(oldEntry.stableId);
						break;
					}
				}
			}

			// Case D: EDITS
			for (FrameEntry& sh : shadow)
			{
				for (FrameEntry& real : entries)
				{
					if (sh.stableId == real.stableId)
					{
						real.frame = sh.frame;
						break;
					}
				}
			}
		}

		ImGui::Dummy(ImVec2());

		SpriteFrame sprite{};
		uiChanged = false;
		if (atlas->Exists(m_SelectedSprite))
			sprite = atlas->GetSpriteFrame(m_SelectedSprite);

		if (UI::DragFloat2("UV", sprite.UV))
			uiChanged = true;

		if (UI::DragFloat2("Size", sprite.Size))
			uiChanged = true;

		if (UI::DragFloat("Frame Time", sprite.FrameTime))
			uiChanged = true;

		if (uiChanged)
			atlas->SetSpriteFrame(m_SelectedSprite, sprite);

		ImGui::Dummy(ImVec2());

		if (m_Mode == AtlasEditorMode::Create)
		{
			int selection = (int)m_GridMode;
			std::vector<const char*> labels{ "size", "rows and cols" };
			if (UI::Combo("grid mode", selection, labels.data(), labels.size()))
			{
				m_GridMode = (GridMode)selection;
			}

			if (m_GridMode == GridMode::Cellsize)
			{
				if (UI::DragInt2("tile size", m_GridTileSize))
					uiChanged = true;
			}
			else if (m_GridMode == GridMode::RowCols)
			{
				if (UI::DragInt("colums", m_Cols))
					uiChanged = true;
				if (UI::DragInt("rows", m_Rows))
					uiChanged = true;
			}

			if (ImGui::Button("create from grid"))
			{
				auto& tex = m_Asset->GetInstance()->GetTexture().Instance();
				bool useSize = m_GridMode == GridMode::Cellsize;
				int x = useSize ? tex->GetWidth() / m_GridTileSize.x : m_Cols;
				int y = useSize ? tex->GetHeight() / m_GridTileSize.y : m_Rows;

				for (auto i : m_SelectedTiles)
				{
					int tileX = i % x;
					int tileY = i / x;

					sprite = {};
					sprite.Size.x = useSize ? (float)m_GridTileSize.x / (float)tex->GetWidth() : 1.f / (float)m_Cols;
					sprite.Size.y = useSize ? (float)m_GridTileSize.y / (float)tex->GetHeight() : 1.f / (float)m_Rows;
					sprite.UV.x = tileX * sprite.Size.x;
					sprite.UV.y = tileY * sprite.Size.y;
					atlas->AddSpriteFrame(sprite);
				}
				m_SelectedTiles.clear();
				m_Mode = AtlasEditorMode::Select;
			}
		}
    }

	glm::vec2 SpriteAtlasEditorPanel::CameraWorldToAtlas(const glm::vec3& world)
	{
		glm::mat4 cam = GetViewport()->GetCamera().GetTransform().GetWorld();
		glm::mat4 invCam = glm::inverse(cam);

		glm::vec4 local = invCam * glm::vec4(world, 1.0f);
		return glm::vec2(local.x, local.y);
	}
}