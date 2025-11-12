#pragma once
#include <string>
#include <typeindex>
#include <unordered_map>
#include <cstdint>
#include <functional>
#include <vector>
#include <optional>
#include "BProperty.h"

namespace Boon
{
    struct ECSLifecycleSystem;
    class GameObject;

    struct BClassMeta
    {
        std::string key;
        std::string value;
    };

    struct BClass
    {
        BClass(std::string _name, std::type_index _type)
            : name(_name), type(_type) {}

        std::string name;
        std::type_index type;
        std::vector<BClassMeta> meta;

        using RegisterFunc = void(*)(ECSLifecycleSystem&);
        using CreateInstanceFn = void* (*)();
        using DestroyInstanceFn = void(*)(void*);
        using AddComponentFn = void* (*)(GameObject&);
        using GetComponentFn = void* (*)(GameObject&);
        using HasComponentFn = bool (*)(GameObject&);
        using RemoveComponentFn = void (*)(GameObject&);

        RegisterFunc registerLifecycle = nullptr;
        CreateInstanceFn createInstance = nullptr;
        DestroyInstanceFn destroyInstance = nullptr;
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

        // quick helpers
        inline const BProperty* FindProperty(const char* propName) const {
            for (auto& p : properties) if (std::string_view(p.name) == propName) return &p;
            return nullptr;
        }

        inline bool HasProperty(const char* propName) const {
            return FindProperty(propName) != nullptr;
        }

        // 🔹 Iteration utilities
        inline void ForEachProperty(const std::function<void(const BProperty&)>& fn) const {
            for (const auto& p : properties)
                fn(p);
        }

        inline void ForEachPropertyMutable(const std::function<void(BProperty&)>& fn) {
            for (auto& p : properties)
                fn(p);
        }

        // 🔹 Generic get/set on instances
        template<typename T>
        inline T& GetValueRef(void* instance, const char* propName) const {
            const BProperty* prop = FindProperty(propName);
            if (!prop)
                throw std::runtime_error("Property not found: " + std::string(propName));
            return *reinterpret_cast<T*>((uint8_t*)instance + prop->offset);
        }

        template<typename T>
        inline const T& GetValueRef(const void* instance, const char* propName) const {
            const BProperty* prop = FindProperty(propName);
            if (!prop)
                throw std::runtime_error("Property not found: " + std::string(propName));
            return *reinterpret_cast<const T*>((const uint8_t*)instance + prop->offset);
        }

        inline const std::vector<BProperty>& GetProperties() const { return properties; }
        inline size_t GetPropertiesCount() const { return properties.size(); }

        // metadata API
        void AddMeta(const std::string& key, const std::string& value = "")
        {
            meta.push_back({ key, value });
        }

        bool HasMeta(const std::string& key) const
        {
            for (auto& m : meta)
                if (m.key == key)
                    return true;
            return false;
        }

        std::optional<std::string> GetMeta(const std::string& key) const
        {
            for (auto& m : meta)
                if (m.key == key)
                    return m.value;
            return std::nullopt;
        }

    private:
        std::vector<BProperty> properties;
    };

    struct BClassRegistry
    {
        static BClassRegistry& Get()
        {
            static BClassRegistry instance;
            return instance;
        }

        void Register(BClass* cls)
        {
            m_Classes[cls->type] = cls;
            for (auto& listener : m_Listeners)
                listener(*cls);
        }

        void AddListener(std::function<void(BClass&)> fn)
        {
            m_Listeners.push_back(std::move(fn));
        }

        BClass* Find(std::type_index t)
        {
            auto it = m_Classes.find(t);
            return (it != m_Classes.end()) ? it->second : nullptr;
        }

        template <typename T>
        BClass* Find()
        {
            return Find(typeid(T));
        }

        template<typename Fn>
        void ForEach(Fn&& fn)
        {
            for (auto& [_, cls] : m_Classes)
                fn(*cls);
        }

        template <typename T>
        T* Instantiate()
        {
            BClass* cls = Find(typeid(T));
            if (cls && cls->createInstance)
                return reinterpret_cast<T*>(cls->createInstance());
            return nullptr;
        }

    private:
        std::unordered_map<std::type_index, BClass*> m_Classes;
        std::vector<std::function<void(BClass&)>> m_Listeners;
    };
}
