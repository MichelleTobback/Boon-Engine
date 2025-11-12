#pragma once
#include "Reflection/BProperty.h"

#include <imgui.h>
#include <string>

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

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100.f);
            ImGui::Text("%s", label.c_str());
            ImGui::NextColumn();

            if (ImGui::Checkbox("##val", &value))
            {
                result = true;
            }

            ImGui::Columns(1);
            ImGui::PopID();

            return result;
        }

		static bool DragFloat(const std::string& label, float& value, float min = 0.1f, float max = 1.f, float step = 0.1f)
		{
            bool result = false;

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100.f);
            ImGui::Text("%s", label.c_str());
            ImGui::NextColumn();

            if (ImGui::DragFloat("##val", &value, step, min, max, "%.2f"))
            {
                result = true;
            }

            ImGui::Columns(1); 
            ImGui::PopID();

            return result;
		}

        static bool DragFloat2(const std::string& label, glm::vec2& value, float min = 0.1f, float max = 1.f, float step = 0.1f)
        {
            bool result = false;

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100.f);
            ImGui::Text("%s", label.c_str());
            ImGui::NextColumn();

            if (ImGui::DragFloat2("##val", &value.x, step, min, max, "%.2f"))
            {
                result = true;
            }

            ImGui::Columns(1); 
            ImGui::PopID();

            return result;
        }

        static bool DragFloat3(const std::string& label, glm::vec3& value, float min = 0.1f, float max = 1.f, float step = 0.1f)
        {
            bool result = false;

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100.f);
            ImGui::Text("%s", label.c_str());
            ImGui::NextColumn();

            if (ImGui::DragFloat3("##val", &value.x, step, min, max, "%.2f"))
            {
                result = true;
            }

            ImGui::Columns(1);
            ImGui::PopID();

            return result;
        }

        static bool DragFloat4(const std::string& label, glm::vec4& value, float min = 0.1f, float max = 1.f, float step = 0.1f)
        {
            bool result = false;

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100.f);
            ImGui::Text("%s", label.c_str());
            ImGui::NextColumn();

            if (ImGui::DragFloat4("##val", &value.x, step, min, max, "%.2f"))
            {
                result = true;
            }

            ImGui::Columns(1);
            ImGui::PopID();

            return result;
        }

        static bool DragInt(const std::string& label, int& value, int min = 0, int max = 100, float speed = 1.f)
        {
            bool result = false;

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100.f);
            ImGui::Text("%s", label.c_str());
            ImGui::NextColumn();

            if (ImGui::DragInt("##val", &value, speed, min, max))
            {
                result = true;
            }

            ImGui::Columns(1);
            ImGui::PopID();

            return result;
        }

        static bool DragInt2(const std::string& label, glm::ivec2& value, int min = 0, int max = 100, float speed = 1.f)
        {
            bool result = false;

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100.f);
            ImGui::Text("%s", label.c_str());
            ImGui::NextColumn();

            if (ImGui::DragInt2("##val", &value.x, speed, min, max))
            {
                result = true;
            }

            ImGui::Columns(1);
            ImGui::PopID();

            return result;
        }

        static bool DragInt3(const std::string& label, glm::ivec3& value, int min = 0, int max = 100, float speed = 1.f)
        {
            bool result = false;

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100.f);
            ImGui::Text("%s", label.c_str());
            ImGui::NextColumn();

            if (ImGui::DragInt3("##val", &value.x, speed, min, max))
            {
                result = true;
            }

            ImGui::Columns(1);
            ImGui::PopID();

            return result;
        }

        static bool DragInt4(const std::string& label, glm::ivec4& value, int min = 0, int max = 100, float speed = 1.f)
        {
            bool result = false;

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100.f);
            ImGui::Text("%s", label.c_str());
            ImGui::NextColumn();

            if (ImGui::DragInt4("##val", &value.x, speed, min, max))
            {
                result = true;
            }

            ImGui::Columns(1);
            ImGui::PopID();

            return result;
        }

        // sliders
        static bool SliderFloat(const std::string& label, float& value, float min = 0.1f, float max = 1.f)
        {
            bool result = false;

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100.f);
            ImGui::Text("%s", label.c_str());
            ImGui::NextColumn();

            if (ImGui::SliderFloat("##val", &value, min, max, "%.2f"))
            {
                result = true;
            }

            ImGui::Columns(1);
            ImGui::PopID();

            return result;
        }

        static bool SliderFloat2(const std::string& label, glm::vec2& value, float min = 0.1f, float max = 1.f)
        {
            bool result = false;

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100.f);
            ImGui::Text("%s", label.c_str());
            ImGui::NextColumn();

            if (ImGui::SliderFloat2("##val", &value.x, min, max, "%.2f"))
            {
                result = true;
            }

            ImGui::Columns(1);
            ImGui::PopID();

            return result;
        }

        static bool SliderFloat3(const std::string& label, glm::vec3& value, float min = 0.1f, float max = 1.f)
        {
            bool result = false;

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100.f);
            ImGui::Text("%s", label.c_str());
            ImGui::NextColumn();

            if (ImGui::SliderFloat3("##val", &value.x, min, max, "%.2f"))
            {
                result = true;
            }

            ImGui::Columns(1);
            ImGui::PopID();

            return result;
        }

        static bool SliderFloat4(const std::string& label, glm::vec4& value, float min = 0.1f, float max = 1.f)
        {
            bool result = false;

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100.f);
            ImGui::Text("%s", label.c_str());
            ImGui::NextColumn();

            if (ImGui::SliderFloat4("##val", &value.x, min, max, "%.2f"))
            {
                result = true;
            }

            ImGui::Columns(1);
            ImGui::PopID();

            return result;
        }

        static bool SliderInt(const std::string& label, int& value, int min = 0, int max = 100)
        {
            bool result = false;

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100.f);
            ImGui::Text("%s", label.c_str());
            ImGui::NextColumn();

            if (ImGui::SliderInt("##val", &value, min, max))
            {
                result = true;
            }

            ImGui::Columns(1);
            ImGui::PopID();

            return result;
        }

        static bool SliderInt2(const std::string& label, glm::ivec2& value, int min = 0, int max = 100)
        {
            bool result = false;

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100.f);
            ImGui::Text("%s", label.c_str());
            ImGui::NextColumn();

            if (ImGui::SliderInt2("##val", &value.x, min, max))
            {
                result = true;
            }

            ImGui::Columns(1);
            ImGui::PopID();

            return result;
        }

        static bool SliderInt3(const std::string& label, glm::ivec3& value, int min = 0, int max = 100)
        {
            bool result = false;

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100.f);
            ImGui::Text("%s", label.c_str());
            ImGui::NextColumn();

            if (ImGui::SliderInt3("##val", &value.x, min, max))
            {
                result = true;
            }

            ImGui::Columns(1);
            ImGui::PopID();

            return result;
        }

        static bool SliderInt4(const std::string& label, glm::ivec4& value, int min = 0, int max = 100)
        {
            bool result = false;

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100.f);
            ImGui::Text("%s", label.c_str());
            ImGui::NextColumn();

            if (ImGui::SliderInt4("##val", &value.x, min, max))
            {
                result = true;
            }

            ImGui::Columns(1);
            ImGui::PopID();

            return result;
        }

        static bool ColorPicker(const std::string& label, glm::vec3& value)
        {
            bool result = false;

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100.f);
            ImGui::Text("%s", label.c_str());
            ImGui::NextColumn();

            ImGuiColorEditFlags flags =
                ImGuiColorEditFlags_DisplayRGB |
                ImGuiColorEditFlags_AlphaPreviewHalf |
                ImGuiColorEditFlags_AlphaBar;

            if (ImGui::ColorEdit4("##val", &value.x, flags))
            {
                result = true;
            }

            ImGui::Columns(1);
            ImGui::PopID();

            return result;
        }

        static bool ColorPicker(const std::string& label, glm::vec4& value)
        {
            bool result = false;

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100.f);
            ImGui::Text("%s", label.c_str());
            ImGui::NextColumn();

            ImGuiColorEditFlags flags =
                ImGuiColorEditFlags_DisplayRGB |
                ImGuiColorEditFlags_AlphaPreviewHalf |
                ImGuiColorEditFlags_AlphaBar;

            if (ImGui::ColorEdit4("##val", &value.x, flags))
            {
                result = true;
            }

            ImGui::Columns(1);
            ImGui::PopID();

            return result;
        }

        static bool Combo(const std::string& label, int& currentItem, const char* const items[], int count)
        {
            bool result = false;

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100.f);
            ImGui::Text("%s", label.c_str());
            ImGui::NextColumn();

            if (ImGui::Combo("##val", &currentItem, items, count))
            {
                result = true;
            }

            ImGui::Columns(1);
            ImGui::PopID();

            return result;
        }
	};
}