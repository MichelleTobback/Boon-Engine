#include "Tools/NewProjectDialog.h"

#include "Tools/ProjectGenerator.h"
#include "Core/EditorContext.h"

#include <imgui.h>
#include <cstring>
#include <filesystem>

#include <shlobj.h>   // SHBrowseForFolder

using namespace Boon;
using namespace BoonEditor;

namespace Boon
{
    static std::filesystem::path OpenFolderDialog()
    {
        BROWSEINFOW bi{};
        bi.lpszTitle = L"Select Project Location";
        bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

        PIDLIST_ABSOLUTE pidl = SHBrowseForFolderW(&bi);
        if (!pidl)
            return {};

        wchar_t path[MAX_PATH];
        if (!SHGetPathFromIDListW(pidl, path))
            return {};

        CoTaskMemFree(pidl);

        return std::filesystem::path(path);
    }
}

namespace
{
    constexpr size_t NameBufferSize = 128;
    constexpr size_t PathBufferSize = 260;
}

NewProjectDialog::NewProjectDialog(const std::string& name, EditorContext* pContext)
    : EditorDialog("Create New Project", pContext)
{
}

std::optional<ProjectGeneratorSettings> NewProjectDialog::ConsumeSettings()
{
    std::optional<ProjectGeneratorSettings> result = m_PendingSettings;
    m_PendingSettings.reset();
    return result;
}

void NewProjectDialog::SetErrorMessage(const std::string& message)
{
    m_ErrorMessage = message;
}

void NewProjectDialog::OnOpen()
{
    m_Settings.Name = "MyGame";
    m_Settings.Location.clear();
    m_Settings.TemplateFolder = "Default";

    //debug
    m_Settings.Location = "D:\\Projects\\BoonProjects";

    m_PendingSettings.reset();
    m_ErrorMessage.clear();
    m_bFocusName = true;
}

bool NewProjectDialog::Validate()
{
    if (m_Settings.Name.empty())
    {
        m_ErrorMessage = "Project name cannot be empty.";
        return false;
    }

    if (m_Settings.Location.empty())
    {
        m_ErrorMessage = "Project location cannot be empty.";
        return false;
    }

    const std::filesystem::path location = m_Settings.Location;
    if (!std::filesystem::exists(location))
    {
        m_ErrorMessage = "Project location does not exist.";
        return false;
    }

    if (!std::filesystem::is_directory(location))
    {
        m_ErrorMessage = "Project location must be a directory.";
        return false;
    }

    if (m_Settings.TemplateFolder.empty())
    {
        m_ErrorMessage = "Template folder cannot be empty.";
        return false;
    }

    m_ErrorMessage.clear();
    return true;
}

void NewProjectDialog::RenderDialog()
{
    if (m_bFocusName)
    {
        ImGui::SetKeyboardFocusHere();
        m_bFocusName = false;
    }

    char nameBuffer[NameBufferSize]{};
    char locationBuffer[PathBufferSize]{};
    char templateBuffer[PathBufferSize]{};

#ifdef _MSC_VER
    strncpy_s(nameBuffer, m_Settings.Name.c_str(), _TRUNCATE);
    strncpy_s(locationBuffer, m_Settings.Location.c_str(), _TRUNCATE);
    strncpy_s(templateBuffer, m_Settings.TemplateFolder.c_str(), _TRUNCATE);
#else
    std::strncpy(nameBuffer, m_Settings.Name.c_str(), NameBufferSize - 1);
    std::strncpy(locationBuffer, m_Settings.Location.c_str(), PathBufferSize - 1);
    std::strncpy(templateBuffer, m_Settings.TemplateFolder.c_str(), PathBufferSize - 1);
#endif

    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
        m_Settings.Name = nameBuffer;

    ImGui::Text("Location");
    ImGui::SameLine();

    ImGui::PushItemWidth(-120); // leave space for button
    if (ImGui::InputText("##Location", locationBuffer, sizeof(locationBuffer)))
        m_Settings.Location = locationBuffer;
    ImGui::PopItemWidth();

    ImGui::SameLine();

    if (ImGui::Button("Browse", ImVec2(100.0f, 0.0f)))
    {
        std::filesystem::path selected = OpenFolderDialog();
        if (!selected.empty())
        {
            m_Settings.Location = selected.string();
        }
    }

    if (ImGui::InputText("Template Folder", templateBuffer, sizeof(templateBuffer)))
        m_Settings.TemplateFolder = templateBuffer;

    ImGui::Spacing();

    if (!m_ErrorMessage.empty())
        ImGui::TextWrapped("%s", m_ErrorMessage.c_str());

    ImGui::Spacing();

    if (ImGui::Button("Create", ImVec2(120.0f, 0.0f)))
    {
        if (m_Settings.TemplateFolder == "Default")
            m_Settings.TemplateFolder = (GetContext().GetCurrentProjectConfig().Editor.EditorResourcesRoot / "Assets/Templates").string();

        if (Validate())
        {
            m_PendingSettings = m_Settings;
            ProjectGenerator::Generate(m_Settings); // move to command
            Close();
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(120.0f, 0.0f)))
    {
        m_PendingSettings.reset();
        m_ErrorMessage.clear();
        Close();
    }
}