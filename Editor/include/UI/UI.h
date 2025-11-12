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
            uint8_t* base = reinterpret_cast<uint8_t*>(pInstance);
            switch (property.typeId)
            {
            case BTypeId::Float:
            {
                float& field = *reinterpret_cast<float*>(base + property.offset);
                float temp = field;

                float min = property.HasMeta("RangeMin") ? std::stof(property.GetMeta("RangeMin").value()) : 0;
                float max = property.HasMeta("RangeMax") ? std::stof(property.GetMeta("RangeMax").value()) : 100;

                if (property.HasMeta("Slider"))
                {
                    if (UI::SliderFloat(property.name, temp, min, max))
                    {
                        field = temp;
                        return true;
                    }
                }
                else
                {
                    if (UI::DragFloat(property.name, temp, min, max))
                    {
                        field = temp;
                        return true;
                    }
                }
            }
            break;
            case BTypeId::Int:
            {
                int& field = *reinterpret_cast<int*>(base + property.offset);
                int temp = field;

                int min = property.HasMeta("RangeMin") ? std::stoi(property.GetMeta("RangeMin").value()) : 0;
                int max = property.HasMeta("RangeMax") ? std::stoi(property.GetMeta("RangeMax").value()) : 100;

                if (property.HasMeta("Slider"))
                {
                    if (UI::SliderInt(property.name, temp, min, max))
                    {
                        field = temp;
                        return true;
                    }
                }
                else
                {
                    if (UI::DragInt(property.name, temp, min, max))
                    {
                        field = temp;
                        return true;
                    }
                }
            }
            break;
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