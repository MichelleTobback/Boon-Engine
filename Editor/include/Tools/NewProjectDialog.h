#pragma once
#include "Panels/EditorDialog.h"
#include "Tools/ProjectGenerator.h"

#include <optional>
#include <string>


namespace BoonEditor
{
    class NewProjectDialog final : public EditorDialog
    {
    public:
        NewProjectDialog(const std::string& name, EditorContext* pContext);
        virtual ~NewProjectDialog() = default;

        std::optional<ProjectGeneratorSettings> ConsumeSettings();

        void SetErrorMessage(const std::string& message);

    protected:
        virtual void OnOpen() override;
        virtual void RenderDialog() override;

    private:
        bool Validate();

    private:
        ProjectGeneratorSettings m_Settings{};
        std::optional<ProjectGeneratorSettings> m_PendingSettings;
        std::string m_ErrorMessage;
        bool m_bFocusName = false;
    };
}