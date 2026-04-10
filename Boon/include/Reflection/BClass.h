#pragma once
#include <string>
#include <typeindex>
#include <unordered_map>
#include <cstdint>
#include <functional>
#include <vector>
#include <optional>
#include <xhash>
#include <stdexcept>
#include <memory>
#include "BProperty.h"
#include "BFunction.h"
#include "Core/Delegate.h"

namespace Boon
{
    struct ECSLifecycleSystem;
    class GameObject;

    class BClassRegistry;
    class NetRepRegistry;

    struct BClassMeta
    {
        std::string key;
        std::string value;
    };

    using BClassID = uint32_t;

    /**
     * @brief Runtime reflection data for a C++ type registered with the engine.
     *
     * Stores function and property reflection metadata and various lifecycle
     * callbacks that may be used by systems such as the ECS lifecycle.
     */
    struct BClass
    {
        BClass(std::string _name, std::type_index _type)
            : name(_name), type(_type)
        {
            std::hash<std::string> hasher;
            hash = (BClassID)hasher(_name);
        }

        BClassID hash;
        std::string name;
        std::type_index type;
        std::vector<BFunction> functions;
        std::vector<BClassMeta> meta;

        using RegisterFunc = void(*)(ECSLifecycleSystem&);
        using UnregisterFunc = void(*)();
        using CreateInstanceFn = void* (*)();
        using DestroyInstanceFn = void(*)(void*);
        using CopyInstanceFn = void (*)(void*, void*);

        using AwakeFn = void (*)(GameObject&);
        using AddComponentFn = void* (*)(GameObject&);
        using GetComponentFn = void* (*)(GameObject&);
        using HasComponentFn = bool (*)(GameObject&);
        using RemoveComponentFn = void (*)(GameObject&);

        RegisterFunc registerLifecycle = nullptr;
        UnregisterFunc unregister = nullptr;
        CreateInstanceFn createInstance = nullptr;
        DestroyInstanceFn destroyInstance = nullptr;
        CopyInstanceFn copyInstance = nullptr;

        AwakeFn awake = nullptr;
        AddComponentFn addComponent = nullptr;
        GetComponentFn getComponent = nullptr;
        HasComponentFn hasComponent = nullptr;
        RemoveComponentFn removeComponent = nullptr;

        uint8_t flags = 0;

        enum FunctionFlags : uint8_t
        {
            None = 0,
            Awake = 1 << 0,
            Update = 1 << 1,
            FixedUpdate = 1 << 2,
            LateUpdate = 1 << 3
        };

        void SetFlag(FunctionFlags f) { flags |= f; }

        // quick helpers
        /**
         * @brief Find property metadata by name.
         *
         * @param propName Name of the property to find.
         * @return Pointer to the BProperty if found, otherwise nullptr.
         */
        inline const BProperty* FindProperty(const char* propName) const {
            for (auto& p : properties) if (std::string_view(p.name) == propName) return &p;
            return nullptr;
        }

        /**
         * @brief Check whether a property with the given name exists.
         */
        inline bool HasProperty(const char* propName) const {
            return FindProperty(propName) != nullptr;
        }

        // 🔹 Iteration utilities
        /**
         * @brief Iterate over all properties (const).
         */
        inline void ForEachProperty(const std::function<void(const BProperty&)>& fn) const {
            for (const auto& p : properties)
                fn(p);
        }

        /**
         * @brief Iterate over all properties (mutable).
         */
        inline void ForEachPropertyMutable(const std::function<void(BProperty&)>& fn) {
            for (auto& p : properties)
                fn(p);
        }

        // 🔹 Generic get/set on instances
        /**
         * @brief Get a reference to a property value on a raw instance pointer.
         *
         * @tparam T Expected type of the property.
         * @param instance Pointer to the object instance memory.
         * @param propName Name of the property.
         * @return Reference to the property value.
         * @throws std::runtime_error if the property is not found.
         */
        template<typename T>
        inline T& GetValueRef(void* instance, const char* propName) const {
            const BProperty* prop = FindProperty(propName);
            if (!prop)
                throw std::runtime_error("Property not found: " + std::string(propName));
            return *reinterpret_cast<T*>((uint8_t*)instance + prop->offset);
        }

        /**
         * @brief Const overload of GetValueRef for read-only access.
         */
        template<typename T>
        inline const T& GetValueRef(const void* instance, const char* propName) const {
            const BProperty* prop = FindProperty(propName);
            if (!prop)
                throw std::runtime_error("Property not found: " + std::string(propName));
            return *reinterpret_cast<const T*>((const uint8_t*)instance + prop->offset);
        }

        /**
         * @brief Access the list of properties for this class.
         */
        inline const std::vector<BProperty>& GetProperties() const { return properties; }

        /**
         * @brief Get the number of reflected properties.
         */
        inline size_t GetPropertiesCount() const { return properties.size(); }
        
        /**
         * @brief Check whether a metadata key exists for this class.
         */
        bool HasMeta(const std::string& key) const
        {
            for (auto& m : meta)
                if (m.key == key)
                    return true;
            return false;
        }

        /**
         * @brief Get a metadata value by key if present.
         */
        std::optional<std::string> GetMeta(const std::string& key) const
        {
            for (auto& m : meta)
                if (m.key == key)
                    return m.value;
            return std::nullopt;
        }

        // -----------------------------------------------------------------
// Function reflection API
// -----------------------------------------------------------------

// Register a function with a known ID (hash of its name)
        /**
         * @brief Register a reflected function for this class.
         *
         * @param id Hashed id of the function name.
         * @param thunk Invocation wrapper used to call the function.
         * @param metaList Optional metadata entries.
         * @param paramsList Optional parameter descriptions.
         */
        inline void AddFunction(
            uint32_t id,
            BFunction::ThunkFn thunk,
            std::initializer_list<BFunctionMeta> metaList = {},
            std::initializer_list<BFunctionParam> paramsList = {})
        {
            BFunction fn;
            fn.id = id;
            fn.thunk = thunk;
            fn.meta.assign(metaList.begin(), metaList.end());
            fn.params.assign(paramsList.begin(), paramsList.end());
            functions.push_back(std::move(fn));
        }

        // Find function by ID
        /**
         * @brief Find a reflected function by hashed id.
         */
        inline BFunction* FindFunction(uint32_t id)
        {
            for (auto& fn : functions)
                if (fn.id == id)
                    return &fn;
            return nullptr;
        }

        /**
         * @brief Const overload of FindFunction.
         */
        inline const BFunction* FindFunction(uint32_t id) const
        {
            for (auto& fn : functions)
                if (fn.id == id)
                    return &fn;
            return nullptr;
        }

        // Invoke by ID
        /**
         * @brief Invoke a reflected function by id on the provided instance.
         *
         * @param instance Pointer to the object instance.
         * @param id Hashed id of the function to invoke.
         * @param args Optional array of Variant arguments.
         * @param argCount Number of arguments provided.
         * @return true if invocation was performed, false otherwise.
         */
        inline bool InvokeById(void* instance, uint32_t id, Variant* args = nullptr, size_t argCount = 0)
        {
            BFunction* fn = FindFunction(id);
            if (!fn || !fn->thunk)
                return false;

            fn->thunk(instance, args, argCount);
            return true;
        }

        // Convenience: invoke by function name (hashed at runtime)
        /**
         * @brief Convenience invocation by function name (hashed at runtime).
         */
        inline bool InvokeByName(void* instance, const std::string& functionName, Variant* args = nullptr, size_t argCount = 0) const
        {
            uint32_t id = FNV1a32(functionName);
            const BFunction* fn = FindFunction(id);
            if (!fn || !fn->thunk)
                return false;

            fn->thunk(const_cast<void*>(instance), args, argCount);
            return true;
        }

        // Quick access
        /**
         * @brief Access reflected function list.
         */
        inline const std::vector<BFunction>& GetFunctions() const { return functions; }

        /**
         * @brief Get the number of reflected functions.
         */
        inline size_t GetFunctionCount() const { return functions.size(); }

        /**
         * @brief Add a property description by offset into the instance.
         *
         * @param propName Name of the property.
         * @param typeName Textual type name.
         * @param offset Byte offset within the instance.
         * @param size Size in bytes of the property.
         * @param typeId Enumerated type id.
         * @param metas Optional metadata entries.
         */
        inline void AddPropertyOffset(const char* propName,
            const char* typeName,
            std::size_t offset,
            std::size_t size,
            BTypeId typeId,
            std::initializer_list<BPropertyMeta> metas = {})
        {
            BProperty p;
            p.name = propName;
            p.typeName = typeName;
            p.offset = offset;
            p.size = size;
            p.typeId = typeId;
            p.meta.assign(metas.begin(), metas.end());
            properties.push_back(std::move(p));
        }

        /**
         * @brief Add a key/value metadata pair to this class.
         */
        void AddMeta(const std::string& key, const std::string& value = "")
        {
            meta.push_back({ key, value });
        }

    private:

        std::vector<BProperty> properties;
    };

    /**
     * @brief Registry holding BClass instances for reflected types.
     *
     * Provides lookup and iteration utilities and notifies listeners when
     * classes are registered.
     */
    class BClassRegistry
    {
    public:
        static BClassRegistry& Get()
        {
            return *s_Instance;
        }

        static void SetRegistry(BClassRegistry* pRegistry)
        {
            s_Instance = pRegistry;
        }

        /**
         * @brief Register a BClass instance with the registry.
         *
         * Invokes any registered listeners after insertion.
         */
        void Register(BClass* cls)
        {
            m_Classes[cls->type] = cls;

            m_Listeners.Invoke(*cls);
        }

        /**
         * @brief Unregister the type T from the registry.
         */
        template<typename T>
        void Unregister()
        {
            BClass* cls = Find<T>();
            if (!cls)
                return;

            cls->unregister();
            m_Classes.erase(cls->type);
        }

        /**
         * @brief Find a BClass by its hashed id.
         */
        BClass* Find(BClassID id)
        {
            auto it = std::find_if(m_Classes.begin(), m_Classes.end(), [id](std::pair<std::type_index, BClass*> pair)
                {
                    return pair.second->hash == id;
                });
            return (it != m_Classes.end()) ? it->second : nullptr;
        }

        /**
         * @brief Find a BClass by its std::type_index.
         */
        BClass* Find(std::type_index t)
        {
            auto it = m_Classes.find(t);
            return (it != m_Classes.end()) ? it->second : nullptr;
        }

        /**
         * @brief Template convenience to find a BClass for type T.
         */
        template <typename T>
        BClass* Find()
        {
            return Find(typeid(T));
        }

        /**
         * @brief Iterate over all registered BClass instances.
         */
        template<typename Fn>
        void ForEach(Fn&& fn)
        {
            for (auto& [_, cls] : m_Classes)
                fn(*cls);
        }

        /**
         * @brief Instantiate an object of type T using the registered createInstance callback.
         *
         * @return Pointer to the newly created instance or nullptr if not available.
         */
        template <typename T>
        T* Instantiate()
        {
            BClass* cls = Find(typeid(T));
            if (cls && cls->createInstance)
                return reinterpret_cast<T*>(cls->createInstance());
            return nullptr;
        }

        /**
         * @brief Access the delegate invoked when classes are registered.
         */
        Delegate<void(BClass&)>& GetListeners() { return m_Listeners; }

    private:
        std::unordered_map<std::type_index, BClass*> m_Classes;
        Delegate<void(BClass&)> m_Listeners;
        static BClassRegistry* s_Instance;
    };
}
