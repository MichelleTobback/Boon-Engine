#pragma once
#include "Scene.h"
#include "Core/Delegate.h"

#include <unordered_map>
#include <memory>
#include <string>

namespace Boon
{
    using SceneChangedDelegate = Delegate<void(Scene&)>;

    class Scene;
    class SceneManager final
    {
    public:
        SceneManager();

        Scene& CreateScene(const std::string& name);

        void UnloadScene(SceneID id);
        void ReloadScene(SceneID id);

        void SetActiveScene(SceneID id, bool setActive = true);
        Scene& GetActiveScene();

        std::vector<Scene*> GetLoadedScenes();
        inline bool IsLoaded(SceneID id) const { return m_Scenes.count(id) != 0; }
        inline bool HasActiveScene() const { return m_ActiveScene != UUID::Null; }

        // Called every frame:
        void Update();
        void FixedUpdate();

        inline SceneChangedDelegate::Handle BindOnSceneChanged(const SceneChangedDelegate::FunctionType& fn) { return m_OnSceneChanged.Bind(fn); }
        inline void UnbindOnSceneChanged(const SceneChangedDelegate::Handle& handle) { return m_OnSceneChanged.Unbind(handle); }

    private:

        SceneID m_ActiveScene = UUID::Null;
        std::unordered_map<SceneID, std::unique_ptr<Scene>> m_Scenes;
        SceneChangedDelegate m_OnSceneChanged{};
    };
}
