#pragma once
#include "Reflection/BProperty.h"
#include "Assets/AssetDatabase.h"
#include <imgui.h>
#include <string>
#include <algorithm>

using namespace Boon;

namespace BoonEditor
{
	class UI
	{
	public:
        static bool Property(const BProperty& property, void* pInstance)
        {
            bool result = false;
            switch (property.typeId)
            {
            case BTypeId::Bool:
                result = BoolProperty(property, pInstance);
                break;

            case BTypeId::Float:
                result = FloatProperty(property, pInstance);
                break;
            case BTypeId::Float2:
                result = Float2Property(property, pInstance);
                break;
            case BTypeId::Float3:
                result = Float3Property(property, pInstance);
                break;
            case BTypeId::Float4:
                result = Float4Property(property, pInstance);
                break;

            case BTypeId::Int:
            case BTypeId::Uint:
            case BTypeId::Int64:
            case BTypeId::Uint64:
                result = IntProperty(property, pInstance);
                break;
            case BTypeId::Int2:
                result = Int2Property(property, pInstance);
                break;
            case BTypeId::Int3:
                result = Int3Property(property, pInstance);
                break;
            case BTypeId::Int4:
                result = Int4Property(property, pInstance);
                break;

            case BTypeId::AssetRef:
                result = AssetRefProperty(property, pInstance);
                break;
            }
            return result;
        }

        static bool BoolProperty(const BProperty& property, void* pInstance)
        {
            std::string name = property.HasMeta("Name") ? property.GetMeta("Name").value() : property.name;
            uint8_t* base = reinterpret_cast<uint8_t*>(pInstance);

            bool& field = *reinterpret_cast<bool*>(base + property.offset);
            bool temp = field;

            if (UI::Checkbox(name, temp))
            {
                field = temp;
                return true;
            }
            return false;
        }

        static bool FloatProperty(const BProperty& property, void* pInstance)
        {
            std::string name = property.HasMeta("Name") ? property.GetMeta("Name").value() : property.name;
            uint8_t* base = reinterpret_cast<uint8_t*>(pInstance);

            float& field = *reinterpret_cast<float*>(base + property.offset);
            float temp = field;

            float min = property.HasMeta("RangeMin") ? std::stof(property.GetMeta("RangeMin").value()) : 0;
            float max = property.HasMeta("RangeMax") ? std::stof(property.GetMeta("RangeMax").value()) : 100;

            if (property.HasMeta("Slider"))
            {
                if (UI::SliderFloat(name, temp, min, max))
                {
                    field = temp;
                    return true;
                }
            }
            else
            {
                if (UI::DragFloat(name, temp, min, max))
                {
                    field = temp;
                    return true;
                }
            }
            return false;
        }

        static bool Float2Property(const BProperty& property, void* pInstance)
        {
            std::string name = property.HasMeta("Name") ? property.GetMeta("Name").value() : property.name;
            uint8_t* base = reinterpret_cast<uint8_t*>(pInstance);

            glm::vec2& field = *reinterpret_cast<glm::vec2*>(base + property.offset);
            glm::vec2 temp = field;

            float min = property.HasMeta("RangeMin") ? std::stof(property.GetMeta("RangeMin").value()) : 0;
            float max = property.HasMeta("RangeMax") ? std::stof(property.GetMeta("RangeMax").value()) : 100;

            if (property.HasMeta("Slider"))
            {
                if (UI::SliderFloat2(name, temp, min, max))
                {
                    field = temp;
                    return true;
                }
            }
            else
            {
                if (UI::DragFloat2(name, temp, min, max))
                {
                    field = temp;
                    return true;
                }
            }
            return false;
        }

        static bool Float3Property(const BProperty& property, void* pInstance)
        {
            std::string name = property.HasMeta("Name") ? property.GetMeta("Name").value() : property.name;
            uint8_t* base = reinterpret_cast<uint8_t*>(pInstance);

            glm::vec3& field = *reinterpret_cast<glm::vec3*>(base + property.offset);
            glm::vec3 temp = field;

            float min = property.HasMeta("RangeMin") ? std::stof(property.GetMeta("RangeMin").value()) : 0;
            float max = property.HasMeta("RangeMax") ? std::stof(property.GetMeta("RangeMax").value()) : 100;

            if (property.HasMeta("Slider"))
            {
                if (UI::SliderFloat3(name, temp, min, max))
                {
                    field = temp;
                    return true;
                }
            }
            else
            {
                if (UI::DragFloat3(name, temp, min, max))
                {
                    field = temp;
                    return true;
                }
            }
            return false;
        }

        static bool Float4Property(const BProperty& property, void* pInstance)
        {
            std::string name = property.HasMeta("Name") ? property.GetMeta("Name").value() : property.name;
            uint8_t* base = reinterpret_cast<uint8_t*>(pInstance);

            glm::vec4& field = *reinterpret_cast<glm::vec4*>(base + property.offset);
            glm::vec4 temp = field;

            float min = property.HasMeta("RangeMin") ? std::stof(property.GetMeta("RangeMin").value()) : 0;
            float max = property.HasMeta("RangeMax") ? std::stof(property.GetMeta("RangeMax").value()) : 100;

            if (property.HasMeta("ColorPicker"))
            {
                if (UI::ColorPicker(name, temp))
                {
                    field = temp;
                    return true;
                }
            }
            else if (property.HasMeta("Slider"))
            {
                if (UI::SliderFloat4(name, temp, min, max))
                {
                    field = temp;
                    return true;
                }
            }
            else
            {
                if (UI::DragFloat4(name, temp, min, max))
                {
                    field = temp;
                    return true;
                }
            }
            return false;
        }

        static bool IntProperty(const BProperty& property, void* pInstance)
        {
            std::string name = property.HasMeta("Name") ? property.GetMeta("Name").value() : property.name;
            uint8_t* base = reinterpret_cast<uint8_t*>(pInstance);

            int& field = *reinterpret_cast<int*>(base + property.offset);
            int temp = field;

            int min = property.HasMeta("RangeMin") ? std::stoi(property.GetMeta("RangeMin").value()) : 0;
            int max = property.HasMeta("RangeMax") ? std::stoi(property.GetMeta("RangeMax").value()) : 100;

            if (property.HasMeta("Slider"))
            {
                if (UI::SliderInt(name, temp, min, max))
                {
                    field = temp;
                    return true;
                }
            }
            else
            {
                if (UI::DragInt(name, temp, min, max))
                {
                    field = temp;
                    return true;
                }
            }
            return false;
        }

        static bool Int2Property(const BProperty& property, void* pInstance)
        {
            std::string name = property.HasMeta("Name") ? property.GetMeta("Name").value() : property.name;
            uint8_t* base = reinterpret_cast<uint8_t*>(pInstance);

            glm::ivec2& field = *reinterpret_cast<glm::ivec2*>(base + property.offset);
            glm::ivec2 temp = field;

            int min = property.HasMeta("RangeMin") ? std::stoi(property.GetMeta("RangeMin").value()) : 0;
            int max = property.HasMeta("RangeMax") ? std::stoi(property.GetMeta("RangeMax").value()) : 100;

            if (property.HasMeta("Slider"))
            {
                if (UI::SliderInt2(name, temp, min, max))
                {
                    field = temp;
                    return true;
                }
            }
            else
            {
                if (UI::DragInt2(name, temp, min, max))
                {
                    field = temp;
                    return true;
                }
            }
            return false;
        }

        static bool Int3Property(const BProperty& property, void* pInstance)
        {
            std::string name = property.HasMeta("Name") ? property.GetMeta("Name").value() : property.name;
            uint8_t* base = reinterpret_cast<uint8_t*>(pInstance);

            glm::ivec3& field = *reinterpret_cast<glm::ivec3*>(base + property.offset);
            glm::ivec3 temp = field;

            int min = property.HasMeta("RangeMin") ? std::stoi(property.GetMeta("RangeMin").value()) : 0;
            int max = property.HasMeta("RangeMax") ? std::stoi(property.GetMeta("RangeMax").value()) : 100;

            if (property.HasMeta("Slider"))
            {
                if (UI::SliderInt3(name, temp, min, max))
                {
                    field = temp;
                    return true;
                }
            }
            else
            {
                if (UI::DragInt3(name, temp, min, max))
                {
                    field = temp;
                    return true;
                }
            }
            return false;
        }

        static bool Int4Property(const BProperty& property, void* pInstance)
        {
            std::string name = property.HasMeta("Name") ? property.GetMeta("Name").value() : property.name;
            uint8_t* base = reinterpret_cast<uint8_t*>(pInstance);

            glm::ivec4& field = *reinterpret_cast<glm::ivec4*>(base + property.offset);
            glm::ivec4 temp = field;

            int min = property.HasMeta("RangeMin") ? std::stoi(property.GetMeta("RangeMin").value()) : 0;
            int max = property.HasMeta("RangeMax") ? std::stoi(property.GetMeta("RangeMax").value()) : 100;

            if (property.HasMeta("Slider"))
            {
                if (UI::SliderInt4(name, temp, min, max))
                {
                    field = temp;
                    return true;
                }
            }
            else
            {
                if (UI::DragInt4(name, temp, min, max))
                {
                    field = temp;
                    return true;
                }
            }
            return false;
        }

        static bool Checkbox(const std::string& label, bool& value)
        {
            bool result = false;

            BeginProperty(label);

            if (ImGui::Checkbox("##val", &value))
            {
                result = true;
            }

            EndProperty();

            return result;
        }

		static bool DragFloat(const std::string& label, float& value, float min = 0.1f, float max = 1.f, float step = 0.1f)
		{
            bool result = false;

            BeginProperty(label);

            if (ImGui::DragFloat("##val", &value, step, min, max, "%.2f"))
            {
                result = true;
            }

            EndProperty();

            return result;
		}

        static bool DragFloat2(const std::string& label, glm::vec2& value, float min = 0.1f, float max = 1.f, float step = 0.1f)
        {
            bool result = false;

            BeginProperty(label);

            if (ImGui::DragFloat2("##val", &value.x, step, min, max, "%.2f"))
            {
                result = true;
            }

            EndProperty();

            return result;
        }

        static bool DragFloat3(const std::string& label, glm::vec3& value, float min = 0.1f, float max = 1.f, float step = 0.1f)
        {
            bool result = false;

            BeginProperty(label);

            if (ImGui::DragFloat3("##val", &value.x, step, min, max, "%.2f"))
            {
                result = true;
            }

            EndProperty();

            return result;
        }

        static bool DragFloat4(const std::string& label, glm::vec4& value, float min = 0.1f, float max = 1.f, float step = 0.1f)
        {
            bool result = false;

            BeginProperty(label);

            if (ImGui::DragFloat4("##val", &value.x, step, min, max, "%.2f"))
            {
                result = true;
            }

            EndProperty();

            return result;
        }

        static bool DragInt(const std::string& label, int& value, int min = 0, int max = 100, float speed = 1.f)
        {
            bool result = false;

            BeginProperty(label);

            if (ImGui::DragInt("##val", &value, speed, min, max))
            {
                result = true;
            }

            EndProperty();

            return result;
        }

        static bool DragInt2(const std::string& label, glm::ivec2& value, int min = 0, int max = 100, float speed = 1.f)
        {
            bool result = false;

            BeginProperty(label);

            if (ImGui::DragInt2("##val", &value.x, speed, min, max))
            {
                result = true;
            }

            EndProperty();

            return result;
        }

        static bool DragInt3(const std::string& label, glm::ivec3& value, int min = 0, int max = 100, float speed = 1.f)
        {
            bool result = false;

            BeginProperty(label);

            if (ImGui::DragInt3("##val", &value.x, speed, min, max))
            {
                result = true;
            }

            EndProperty();

            return result;
        }

        static bool DragInt4(const std::string& label, glm::ivec4& value, int min = 0, int max = 100, float speed = 1.f)
        {
            bool result = false;

            BeginProperty(label);

            if (ImGui::DragInt4("##val", &value.x, speed, min, max))
            {
                result = true;
            }

            EndProperty();

            return result;
        }

        // sliders
        static bool SliderFloat(const std::string& label, float& value, float min = 0.1f, float max = 1.f)
        {
            bool result = false;

            BeginProperty(label);

            if (ImGui::SliderFloat("##val", &value, min, max, "%.2f"))
            {
                result = true;
            }

            EndProperty();

            return result;
        }

        static bool SliderFloat2(const std::string& label, glm::vec2& value, float min = 0.1f, float max = 1.f)
        {
            bool result = false;

            BeginProperty(label);

            if (ImGui::SliderFloat2("##val", &value.x, min, max, "%.2f"))
            {
                result = true;
            }

            EndProperty();

            return result;
        }

        static bool SliderFloat3(const std::string& label, glm::vec3& value, float min = 0.1f, float max = 1.f)
        {
            bool result = false;

            BeginProperty(label);

            if (ImGui::SliderFloat3("##val", &value.x, min, max, "%.2f"))
            {
                result = true;
            }

            EndProperty();

            return result;
        }

        static bool SliderFloat4(const std::string& label, glm::vec4& value, float min = 0.1f, float max = 1.f)
        {
            bool result = false;

            BeginProperty(label);

            if (ImGui::SliderFloat4("##val", &value.x, min, max, "%.2f"))
            {
                result = true;
            }

            EndProperty();

            return result;
        }

        static bool SliderInt(const std::string& label, int& value, int min = 0, int max = 100)
        {
            bool result = false;

            BeginProperty(label);

            if (ImGui::SliderInt("##val", &value, min, max))
            {
                result = true;
            }

            EndProperty();

            return result;
        }

        static bool SliderInt2(const std::string& label, glm::ivec2& value, int min = 0, int max = 100)
        {
            bool result = false;

            BeginProperty(label);

            if (ImGui::SliderInt2("##val", &value.x, min, max))
            {
                result = true;
            }

            EndProperty();

            return result;
        }

        static bool SliderInt3(const std::string& label, glm::ivec3& value, int min = 0, int max = 100)
        {
            bool result = false;

            BeginProperty(label);

            if (ImGui::SliderInt3("##val", &value.x, min, max))
            {
                result = true;
            }

            EndProperty();

            return result;
        }

        static bool SliderInt4(const std::string& label, glm::ivec4& value, int min = 0, int max = 100)
        {
            bool result = false;

            BeginProperty(label);

            if (ImGui::SliderInt4("##val", &value.x, min, max))
            {
                result = true;
            }

            EndProperty();

            return result;
        }

        static bool ColorPicker(const std::string& label, glm::vec3& value)
        {
            bool result = false;

            BeginProperty(label);

            ImGuiColorEditFlags flags =
                ImGuiColorEditFlags_DisplayRGB |
                ImGuiColorEditFlags_AlphaPreviewHalf |
                ImGuiColorEditFlags_AlphaBar;

            if (ImGui::ColorEdit4("##val", &value.x, flags))
            {
                result = true;
            }

            EndProperty();

            return result;
        }

        static bool ColorPicker(const std::string& label, glm::vec4& value)
        {
            bool result = false;

            BeginProperty(label);

            ImGuiColorEditFlags flags =
                ImGuiColorEditFlags_DisplayRGB |
                ImGuiColorEditFlags_AlphaPreviewHalf |
                ImGuiColorEditFlags_AlphaBar;

            if (ImGui::ColorEdit4("##val", &value.x, flags))
            {
                result = true;
            }
            EndProperty();

            return result;
        }

        static bool Combo(const std::string& label, int& currentItem, const char* const items[], int count)
        {
            bool result = false;

            BeginProperty(label);
            if (ImGui::Combo("##val", &currentItem, items, count))
            {
                result = true;
            }
            EndProperty();

            return result;
        }

        static bool Field(const std::string& label, std::string& value)
        {
            bool result = false;

            BeginProperty(label);
            char buffer[256];
            strncpy(buffer, value.c_str(), sizeof(buffer));
            buffer[sizeof(buffer) - 1] = '\0';

            if (ImGui::InputText("##val", buffer, sizeof(buffer)))
            {
                value = buffer;
                result = true;
            }
            EndProperty();

            return result;
        }

        static bool Input(const std::string& label, int& value)
        {
            bool result = false;

            BeginProperty(label);
            if (ImGui::InputInt("##val", &value))
            {
                result = true;
            }
            EndProperty();

            return result;
        }

        static bool InputDigits(const std::string& label, int& value, int digits)
        {
            bool changed = false;

            BeginProperty(label);

            digits = std::clamp(digits, 1, 10);

            ImGui::PushID(label.c_str());

            // Layout calc
            float fullWidth = ImGui::GetContentRegionMax().x * 0.65f;
            float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
            float digitWidth = (fullWidth - spacing * (digits - 1)) / digits;

            // --- Persistent digit storage ---
            ImGuiStorage* storage = ImGui::GetStateStorage();
            ImGuiID base = ImGui::GetID("digits");

            // Whether any digit is currently being edited
            bool anyActive = false;

            // Load stored digits, but only if not active
            std::vector<int> digitVals(digits);
            bool stored = storage->GetBool(base + 99999, false);

            if (!stored) {
                // initialize digit storage
                int temp = abs(value);
                for (int i = digits - 1; i >= 0; --i) {
                    digitVals[i] = temp % 10;
                    temp /= 10;
                    storage->SetInt(base + i, digitVals[i]);
                }
                storage->SetBool(base + 99999, true);
            }
            else {
                // load current stored digits
                for (int i = 0; i < digits; ++i)
                    digitVals[i] = storage->GetInt(base + i, 0);
            }

            // --- Draw digits ---
            ImGui::BeginGroup();
            for (int i = 0; i < digits; ++i)
            {
                ImGui::PushID(i);

                ImGui::SetNextItemWidth(digitWidth);
                int v = digitVals[i];
                if (ImGui::DragInt("##val", &v, 0.6, 0, 9))
                {
                    v = std::clamp(v, 0, 9);
                    digitVals[i] = v;
                    storage->SetInt(base + i, v);
                    changed = true;
                }

                if (ImGui::IsItemActive())
                    anyActive = true;

                ImGui::PopID();
                if (i < digits - 1)
                    ImGui::SameLine(0.0f, spacing);
            }
            ImGui::EndGroup();

            // --- Only recompute value when NONE are active ---
            if (!anyActive)
            {
                int newValue = 0;
                for (int i = 0; i < digits; ++i)
                    newValue = newValue * 10 + digitVals[i];

                if (newValue != value)
                {
                    value = newValue;
                    changed = true;
                }
            }

            ImGui::PopID();
            EndProperty();

            return changed;
        }

        static bool AssetRefProperty(const BProperty& property, void* pInstance)
        {
            std::string label = property.HasMeta("Name")
                ? property.GetMeta("Name").value()
                : property.name;

            uint8_t* base = reinterpret_cast<uint8_t*>(pInstance);
            AssetHandle* handlePtr = reinterpret_cast<AssetHandle*>(base + property.offset);

            AssetHandle current = *handlePtr;
            bool changed = false;

            BeginProperty(label);

            //
            // Resolve current label (filename only)
            //
            std::string currentLabel;
            if (current != 0 && AssetDatabase::Get().Exists(current))
            {
                std::string fullPath = AssetDatabase::Get().GetPath(current);
                currentLabel = std::filesystem::path(fullPath).filename().string();
            }
            else
            {
                currentLabel = "<None>";
            }

            float clearButtonWidth = 30.0f;
            ImVec2 buttonSize(ImGui::GetContentRegionAvail().x - clearButtonWidth, 0);

            //
            // MAIN BUTTON (display asset)
            //
            bool pressed = ImGui::Button(currentLabel.c_str(), buttonSize);

            if (pressed)
                ImGui::OpenPopup("AssetPicker");

            //
            // DRAG & DROP TARGET — must be attached *right here*
            //
            if (ImGui::BeginDragDropTarget())
            {
                const ImGuiPayload* payload = ImGui::GetDragDropPayload();
                bool compatible = false;

                if (payload && payload->IsDataType("ASSET_HANDLE"))
                {
                    AssetHandle hovered = *(const AssetHandle*)payload->Data;

                    const AssetMeta* meta =
                        AssetImporterRegistry::Get().GetRegistry()->Get(hovered);

                    if (meta && AssetMatchesPropertyType(property, meta->type))
                        compatible = true;
                }

                // ✔ Accept the payload if compatible
                if (compatible)
                {
                    if (const ImGuiPayload* accepted =
                        ImGui::AcceptDragDropPayload("ASSET_HANDLE"))
                    {
                        AssetHandle dropped = *(const AssetHandle*)accepted->Data;
                        *handlePtr = dropped;
                        changed = true;
                    }
                }

                ImGui::EndDragDropTarget();
            }

            //
            // HIGHLIGHT DURING DRAG HOVER
            //
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
            {
                const ImGuiPayload* payload = ImGui::GetDragDropPayload();

                if (payload && payload->IsDataType("ASSET_HANDLE"))
                {
                    AssetHandle hovered = *(const AssetHandle*)payload->Data;
                    const AssetMeta* meta =
                        AssetImporterRegistry::Get().GetRegistry()->Get(hovered);

                    bool compatible = (meta && AssetMatchesPropertyType(property, meta->type));

                    if (compatible)
                    {
                        ImGui::GetWindowDrawList()->AddRectFilled(
                            ImGui::GetItemRectMin(),
                            ImGui::GetItemRectMax(),
                            IM_COL32(60, 180, 80, 80)); // Soft green
                    }
                    else
                    {
                        ImGui::GetWindowDrawList()->AddRectFilled(
                            ImGui::GetItemRectMin(),
                            ImGui::GetItemRectMax(),
                            IM_COL32(180, 60, 60, 80)); // Soft red
                    }
                }
            }

            //
            // CLEAR BUTTON
            //
            ImGui::SameLine();
            if (ImGui::Button("X", ImVec2(clearButtonWidth, 0)))
            {
                *handlePtr = 0;
                changed = true;
            }

            //
            // ASSET PICKER POPUP
            //
            if (ImGui::BeginPopup("AssetPicker"))
            {
                static char filter[128] = { 0 };
                ImGui::InputText("Search", filter, sizeof(filter));
                ImGui::Separator();

                const auto& all = AssetImporterRegistry::Get().GetRegistry()->GetAll();

                for (auto& it : all)
                {
                    const UUID& uuid = it.first;
                    const AssetMeta& meta = it.second;

                    if (!AssetMatchesPropertyType(property, meta.type))
                        continue;

                    std::string path = AssetDatabase::Get().GetPath(meta.uuid);

                    if (!ContainsCaseInsensitive(path, filter))
                        continue;

                    std::string filename = std::filesystem::path(path).filename().string();
                    bool selected = (uuid == current);

                    if (ImGui::Selectable(filename.c_str(), selected))
                    {
                        *handlePtr = uuid;
                        changed = true;
                        ImGui::CloseCurrentPopup();
                    }
                }

                ImGui::Separator();

                if (ImGui::Selectable("<None>"))
                {
                    *handlePtr = 0;
                    changed = true;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            EndProperty();
            return changed;
        }

        template<typename T>
        static bool List(const std::string& label,
            std::vector<T>& list,
            int& selected,
            const std::function<bool(const std::string&, T&)>& drawItem)
        {
            bool changed = false;
            const float rowHeight = 22;
            static float listHeight = 250.0f;

            BeginProperty(label);

            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));

            float listPosY = ImGui::GetCursorScreenPos().y;
            ImGui::BeginChild("##scroll", ImVec2(0, listHeight), true);

            int dragSrc = -1;
            int dragDst = -1;
            int itemToRemove = -1;

            const float fullWidth = ImGui::GetContentRegionAvail().x;

            // ───────────────────────────────────────────
            // LOOP ITEMS
            // ───────────────────────────────────────────
            for (int i = 0; i < list.size(); ++i)
            {
                ImGui::PushID(i);
                ImGui::BeginGroup();

                // Selectable full-width row
                bool isSelected = (selected == i);
                if (UI::Selectable("##select_row", isSelected,
                    glm::vec2(fullWidth - 28, rowHeight)))
                {
                    selected = i; // set selected index
                }

                // Drag source
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                {
                    dragSrc = i;
                    ImGui::SetDragDropPayload("REORDER_ITEM", &i, sizeof(int));
                    ImGui::Text("Move Item");
                    ImGui::EndDragDropSource();
                }

                // Remove button
                ImGui::SameLine(fullWidth - 24);
                if (ImGui::Button("X"))
                    itemToRemove = i;

                // Draw item content inside the row
                //ImGui::SetCursorPosY(ImGui::GetCursorPosY() - rowHeight + 6);
                changed |= drawItem("Item", list[i]);

                ImGui::EndGroup();

                // Drop target between items
                float dropHeight = 3;
                ImVec2 dropMin = ImGui::GetCursorScreenPos();
                ImVec2 dropMax(dropMin.x + fullWidth, dropMin.y + dropHeight);

                ImGui::InvisibleButton("drop_target", ImVec2(fullWidth, dropHeight));

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload =
                        ImGui::AcceptDragDropPayload("REORDER_ITEM"))
                    {
                        dragDst = i + 1;
                    }
                    ImGui::EndDragDropTarget();
                }

                // Visual drop highlight
                if (ImGui::IsItemHovered() && ImGui::GetDragDropPayload() &&
                    strcmp(ImGui::GetDragDropPayload()->DataType, "REORDER_ITEM") == 0)
                {
                    ImU32 dropColor = ImGui::GetColorU32(ImGuiCol_HeaderActive);

                    ImGui::GetWindowDrawList()->AddRectFilled(
                        dropMin, dropMax, dropColor);
                }

                ImGui::Separator();
                ImGui::PopID();
            }

            ImGui::EndChild();
            ImGui::PopStyleVar(2);

            // ───────────────────────────────────────────
            // HEIGHT DRAG HANDLE
            // ───────────────────────────────────────────
            {
                const float gripHeight = 6.0f;
                const float thinHeight = 1.0f;

                float width = fullWidth;

                // --- Thin hitbox for interaction ---
                ImGui::InvisibleButton("##row_height_grip", ImVec2(width, thinHeight));

                bool hovered = ImGui::IsItemHovered();
                bool held = ImGui::IsItemActive() && ImGui::IsMouseDown(0);

                // Change cursor when hovered
                if (hovered || held)
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);

                // --- Draw actual visual bar (1px or 6px) ---
                ImVec2 min = ImGui::GetItemRectMin();
                ImVec2 max = ImVec2(min.x + width, min.y + (hovered || held ? gripHeight : thinHeight));

                ImU32 col = ImGui::GetColorU32(hovered || held ? ImGuiCol_FrameBgHovered
                    : ImGuiCol_FrameBg);

                ImGui::GetWindowDrawList()->AddRectFilled(min, max, col, 2.0f);

                // --- Dragging logic ---
                static bool dragging = false;

                if (held) dragging = true;
                else if (!ImGui::IsMouseDown(0)) dragging = false;

                if (dragging)
                {
                    float mouseY = ImGui::GetIO().MousePos.y;
                    listHeight = mouseY - listPosY;

                    // clamp
                    listHeight = std::clamp(listHeight, 50.0f, 800.0f);
                }
            }

            // ───────────────────────────────────────────
            // Add button
            // ───────────────────────────────────────────
            if (ImGui::Button("+ Add", ImVec2(-1, 0)))
            {
                list.emplace_back(T{});
                changed = true;

                // Auto-select new item
                selected = (int)list.size() - 1;
            }

            // ───────────────────────────────────────────
            // PERFORM REMOVE
            // ───────────────────────────────────────────
            if (itemToRemove != -1)
            {
                list.erase(list.begin() + itemToRemove);
                changed = true;

                // Fix selection
                if (selected == itemToRemove)
                    selected = -1;
                else if (selected > itemToRemove)
                    selected--;
            }

            // ───────────────────────────────────────────
            // PERFORM REORDER
            // ───────────────────────────────────────────
            if (dragSrc != -1 && dragDst != -1 && dragSrc != dragDst)
            {
                dragDst = std::clamp(dragDst, 0, (int)list.size());

                T item = list[dragSrc];
                list.erase(list.begin() + dragSrc);

                if (dragDst > dragSrc)
                    dragDst--;

                list.insert(list.begin() + dragDst, item);

                // Adjust selected index
                if (selected == dragSrc)
                    selected = dragDst;
                else if (selected >= std::min(dragSrc, dragDst) &&
                    selected <= std::max(dragSrc, dragDst))
                {
                    // Keeps selection moving with item block
                    if (dragSrc < dragDst) selected--;
                    else selected++;
                }

                changed = true;
            }

            EndProperty();
            return changed;
        }

        static bool Selectable(const char* id, bool selected, const glm::vec2& size = {}, float rounding = 6.0f)
            {
                ImGuiWindow* window = ImGui::GetCurrentWindow();
                if (window->SkipItems)
                    return false;

                // Determine final size
                ImVec2 pos = ImGui::GetCursorScreenPos();
                ImVec2 itemSize = ImGui::CalcItemSize(ImVec2(size.x, size.y), 0, ImGui::GetTextLineHeight());

                ImRect rect(pos.x, pos.y, pos.x + itemSize.x, pos.y + itemSize.y);
                bool hovered = ImGui::IsMouseHoveringRect(rect.Min, rect.Max);
                bool pressed = hovered && ImGui::IsMouseClicked(0);

                // --- PICK COLORS FROM IMGUI STYLE ---
                ImU32 col =
                    selected ? ImGui::GetColorU32(ImGuiCol_Header) :
                    hovered ? ImGui::GetColorU32(ImGuiCol_HeaderHovered) :
                    ImGui::GetColorU32(ImGuiCol_FrameBg);

                // --- DRAW BACKGROUND WITH ROUNDING ---
                window->DrawList->AddRectFilled(rect.Min, rect.Max, col, rounding);

                // --- INVISIBLE INTERACTION BOX ---
                ImGui::InvisibleButton(id, itemSize);

                return pressed;
            }

        static bool Selectable(const char* id, bool selected,
            const glm::vec4& colSelected, const glm::vec4& colHover, const glm::vec4& colNormal, 
            const glm::vec2& size = {}, float rounding = 6.0f)
        {
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            if (window->SkipItems)
                return false;

            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImVec2 itemSize = ImGui::CalcItemSize(ImVec2(size.x, size.y), 0, ImGui::GetTextLineHeight());

            ImRect rect(pos.x, pos.y, pos.x + itemSize.x, pos.y + itemSize.y);
            bool hovered = ImGui::IsMouseHoveringRect(rect.Min, rect.Max);
            bool pressed = hovered && ImGui::IsMouseClicked(0);

            // Choose bg color
            glm::vec4 color =
                selected ? colSelected :
                hovered ? colHover :
                colNormal;

            color *= 255.f;

            ImU32 col = IM_COL32((uint32_t)color.x, (uint32_t)color.y, (uint32_t)color.z, (uint32_t)color.w);

            // Rounded background
            window->DrawList->AddRectFilled(rect.Min, rect.Max, col, rounding);

            // Create an invisible button so item is interactive
            ImGui::InvisibleButton(id, itemSize);

            return pressed;
        }


        private:
            static void BeginProperty(const std::string& label)
            {
                ImGui::PushID(label.c_str());

                float panelWidth = ImGui::GetContentRegionAvail().x;

                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, panelWidth * 0.3f);
                ImGui::Text("%s", label.c_str());
                ImGui::NextColumn();

                ImGui::PushItemWidth(panelWidth * 0.65f);
            }
            static void EndProperty()
            {
                ImGui::PopItemWidth();
                ImGui::Columns(1);
                ImGui::PopID();
            }

            static bool AssetMatchesPropertyType(const BProperty& prop, AssetType assetType)
            {
                // Extract C++ type name from "AssetRef<Texture2DAsset>"
                std::string type = prop.typeName;

                auto start = type.find('<');
                auto end = type.find('>');

                if (start == std::string::npos || end == std::string::npos)
                    return true; // fallback no filtering

                std::string assetClass = type.substr(start + 1, end - start - 1);

                // Look up asset traits
                // You may need a map: "Texture2DAsset" -> AssetType::Texture
                AssetType expected = AssetTypeFromClassName(assetClass);

                return expected == assetType;
            }

            static AssetType AssetTypeFromClassName(const std::string& name)
            {
                if (name == "Texture2DAsset") return AssetType::Texture;
                if (name == "ShaderAsset") return AssetType::Shader;
                if (name == "SpriteAtlasAsset") return AssetType::SpriteAtlas;
                if (name == "TilemapAsset") return AssetType::Tilemap;
                // ...

                return AssetType::None;
            }

            static std::string ToLower(const std::string& s)
            {
                std::string out = s;
                for (auto& c : out) c = (char)std::tolower(c);
                return out;
            }

            static bool ContainsCaseInsensitive(const std::string& str, const std::string& filter)
            {
                if (filter.empty()) return true;

                std::string lowerStr = ToLower(str);
                std::string lowerFilter = ToLower(filter);

                return lowerStr.find(lowerFilter) != std::string::npos;
            }
	};
}