#include "Scene/SceneManager.h"

#include "Event/EventBus.h"
#include "Event/SceneEvents.h"

#include "Core/ServiceLocator.h"

namespace Boon
{
    SceneManager::SceneManager()
    {
        CreateScene("empty");
    }

    Scene& SceneManager::CreateScene(const std::string& name)
    {
        auto scene = std::unique_ptr<Scene>(new Scene(name));

        SceneID id = scene->GetID();
        m_Scenes[id] = std::move(scene);

        if (m_ActiveScene == UUID::Null)
            m_ActiveScene = id;

        return *m_Scenes[id];
    }

    void SceneManager::UnloadScene(SceneID id)
    {
        if (m_ActiveScene == id)
            m_ActiveScene = UUID::Null;

        m_Scenes.erase(id);
    }

    void SceneManager::SetActiveScene(SceneID id)
    {
        m_ActiveScene = id;

        EventBus& eventBus = ServiceLocator::Get<EventBus>();
        eventBus.Post(SceneChangedEvent(id));
    }

    Scene& SceneManager::GetActiveScene()
    {
        return *m_Scenes.at(m_ActiveScene);
    }

    std::vector<Scene*> SceneManager::GetLoadedScenes()
    {
        std::vector<Scene*> result;
        for (auto& [id, ptr] : m_Scenes)
            result.push_back(ptr.get());
        return result;
    }

    void SceneManager::Update()
    {
        for (auto& [id, scene] : m_Scenes)
            scene->Update();
    }

    void SceneManager::FixedUpdate()
    {
        for (auto& [id, scene] : m_Scenes)
            scene->FixedUpdate();
    }

    void SceneManager::LateUpdate()
    {
        for (auto& [id, scene] : m_Scenes)
            scene->LateUpdate();
    }
}