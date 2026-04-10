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
        /**
         * @brief Construct a new SceneManager.
         *
         * Responsible for creating, loading and switching scenes.
         */
        SceneManager();

        /**
         * @brief Shutdown the scene manager and unload all scenes.
         */
        void Shutdown();

        /**
         * @brief Create and register a new scene with the given name.
         * @param name Name for the new scene.
         * @return Reference to the created Scene.
         */
        Scene& CreateScene(const std::string& name);

        /**
         * @brief Unload the scene identified by id.
         * @param id The SceneID to unload.
         */
        void UnloadScene(SceneID id);

        /**
         * @brief Unload all scenes
         */
        void UnloadAll();

        /**
         * @brief Reload the scene identified by id from its source.
         * @param id The SceneID to reload.
         */
        void ReloadScene(SceneID id);

        /**
         * @brief Set the active scene by id.
         * @param id SceneID to activate.
         * @param setActive If false, will deactivate the active scene.
         */
        void SetActiveScene(SceneID id, bool setActive = true);

        /**
         * @brief Get the currently active scene.
         * @return Reference to the active Scene.
         */
        Scene& GetActiveScene();

        /**
         * @brief Retrieve a list of pointers to all loaded scenes.
         * @return Vector of loaded Scene pointers.
         */
        /**
         * @brief Retrieve a list of pointers to all loaded scenes.
         *
         * @return Vector of loaded Scene pointers.
         */
        std::vector<Scene*> GetLoadedScenes();

        /**
         * @brief Check whether a scene with the given id is registered with the manager.
         *
         * This performs a lookup in the internal scene registry. See implementation for
         * exact lookup semantics.
         *
         * @param id SceneID to check for.
         * @return true if the scene id is present in the manager's registry, false otherwise.
         */
        inline bool IsLoaded(SceneID id) const { return m_Scenes.count(id) != 0; }

        /**
         * @brief Check whether an active scene is set on the manager.
         *
         * The manager stores the active scene id in `m_ActiveScene`; this returns true
         * when that id is not equal to `UUID::Null`.
         *
         * @return true if there is an active scene id, false otherwise.
         */
        inline bool HasActiveScene() const { return m_ActiveScene != UUID::Null; }

        // Called every frame:
        /**
         * @brief Run per-frame update for loaded scenes.
         */
        void Update();

        /**
         * @brief Run fixed-timestep updates (physics) for scenes.
         */
        void FixedUpdate();

        /**
         * @brief Bind a callback to the scene-changed delegate.
         *
         * The provided function must match the delegate signature (`void(Scene&)`).
         * The returned handle can be used to unbind the callback.
         *
         * @param fn Function to bind.
         * @return Handle representing the binding.
         */
        inline SceneChangedDelegate::Handle BindOnSceneChanged(const SceneChangedDelegate::FunctionType& fn) { return m_OnSceneChanged.Bind(fn); }

        /**
         * @brief Unbind a previously bound scene-changed callback.
         *
         * @param handle The handle returned by `BindOnSceneChanged` for the binding to remove.
         */
        inline void UnbindOnSceneChanged(const SceneChangedDelegate::Handle& handle) { return m_OnSceneChanged.Unbind(handle); }

    private:

        SceneID m_ActiveScene = UUID::Null;
        std::unordered_map<SceneID, std::unique_ptr<Scene>> m_Scenes;
        SceneChangedDelegate m_OnSceneChanged{};
    };
}
