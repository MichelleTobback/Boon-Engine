#pragma once
#include "Scene.h"

#include <unordered_map>
#include <memory>
#include <string>

namespace Boon
{
    class Scene;
    class SceneManager final
    {
    public:
        SceneManager();

        Scene& CreateScene(const std::string& name);

        void UnloadScene(SceneID id);
        void ReloadScene(SceneID id);

        void SetActiveScene(SceneID id);
        Scene& GetActiveScene();

        std::vector<Scene*> GetLoadedScenes();
        inline bool IsLoaded(SceneID id) const { return m_Scenes.count(id) != 0; }
        inline bool HasActiveScene() const { return m_ActiveScene != UUID::Null; }

        // Called every frame:
        void Update();
        void FixedUpdate();

    private:

        SceneID m_ActiveScene = UUID::Null;
        std::unordered_map<SceneID, std::unique_ptr<Scene>> m_Scenes;
    };
}
