#include "Scene/SceneSerializer.h"
#include "Component/NameComponent.h"
#include "Component/SceneComponent.h"
#include "Asset/Asset.h"
#include "Asset/AssetRef.h"

#include "Scene/GameObject.h"
#include "Reflection/BClass.h"

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace Boon;

Boon::SceneSerializer::SceneSerializer(Scene& scene)
	: m_Context(scene){}

void Boon::SceneSerializer::Serialize(const std::string& dst)
{
    json j;
    j["scene"] = m_Context.m_Name;

    j["objects"] = json::array();   // array to hold all game objects

    m_Context.ForeachGameObject([&](GameObject gameObject)
        {
            json jObj;
            jObj["uuid"] = static_cast<uint64_t>(gameObject.GetUUID());
            jObj["components"] = json::object();

            json& scene = jObj["components"]["scene component"];
            scene["id"] = 0;
            scene["parent"] = gameObject.GetParent().IsValid() ? (uint64_t)gameObject.GetParent().GetUUID() : 0u;
            scene["children"] = json::array();
            for (auto child : gameObject.GetChildren())
            {
                scene["children"].push_back((uint64_t)child.GetUUID());
            }

            // Iterate over all registered component classes
            BClassRegistry::Get().ForEach([&](const BClass& cls)
                {
                    if (!gameObject.HasComponentByClass(&cls))
                        return;

                    json jComponent = json::object();
                    jComponent["id"] = cls.hash;
                    uint8_t* base = reinterpret_cast<uint8_t*>(gameObject.GetComponentByClass(&cls));
                    // Iterate over component properties
                    cls.ForEachProperty([&](const BProperty& prop)
                        {
                            json jValue;

                            // ---- VALUE EXTRACTION ----
                            switch (prop.typeId)
                            {
                            case BTypeId::Int:
                                jValue = *reinterpret_cast<int32_t*>(base + prop.offset);
                                break;

                            case BTypeId::Uint:
                                jValue = *reinterpret_cast<uint32_t*>(base + prop.offset);
                                break;

                            case BTypeId::Int64:
                                jValue = *reinterpret_cast<int64_t*>(base + prop.offset);
                                break;

                            case BTypeId::Uint64:
                                jValue = *reinterpret_cast<uint64_t*>(base + prop.offset);
                                break;

                            case BTypeId::Float:
                                jValue = *reinterpret_cast<float*>(base + prop.offset);
                                break;

                            case BTypeId::Bool:
                                jValue = *reinterpret_cast<bool*>(base + prop.offset);
                                break;

                            case BTypeId::String:
                                jValue = *reinterpret_cast<std::string*>(base + prop.offset);
                                break;

                            case BTypeId::Float2:
                            {
                                auto v = *reinterpret_cast<glm::vec2*>(base + prop.offset);
                                jValue = { v.x, v.y };
                                break;
                            }
                            case BTypeId::Float3:
                            {
                                auto v = *reinterpret_cast<glm::vec3*>(base + prop.offset);
                                jValue = { v.x, v.y, v.z };
                                break;
                            }
                            case BTypeId::Float4:
                            {
                                auto v = *reinterpret_cast<glm::vec4*>(base + prop.offset);
                                jValue = { v.x, v.y, v.z, v.w };
                                break;
                            }
                            case BTypeId::Int2:
                            {
                                auto v = *reinterpret_cast<glm::ivec2*>(base + prop.offset);
                                jValue = { v.x, v.y };
                                break;
                            }
                            case BTypeId::Int3:
                            {
                                auto v = *reinterpret_cast<glm::ivec3*>(base + prop.offset);
                                jValue = { v.x, v.y, v.z };
                                break;
                            }
                            case BTypeId::Int4:
                            {
                                auto v = *reinterpret_cast<glm::ivec4*>(base + prop.offset);
                                jValue = { v.x, v.y, v.z, v.w };
                                break;
                            }
                            case BTypeId::AssetRef:
                            {
                                AssetHandle* pAsset = reinterpret_cast<AssetHandle*>(base + prop.offset);
                                jValue = static_cast<uint64_t>(*pAsset);
                                break;
                            }
                            default:
                                jValue = nullptr;
                                break;
                            }

                            jComponent[prop.name] = jValue;
                        });

                    jObj["components"][cls.name] = jComponent;
                });

            j["objects"].push_back(jObj);
        });

    std::ofstream file(dst);
    file << j.dump(4);
}


void Boon::SceneSerializer::Deserialize(const std::string& src)
{
    std::ifstream file(src);
    if (!file.is_open())
    {
        //BOON_ERROR("Could not open scene file: {}", src);
        return;
    }

    json j;
    file >> j;

    // Load scene name
    m_Context.m_Name = j["scene"].get<std::string>();

    // Load objects
    for (auto& jObj : j["objects"])
    {
        uint64_t uuid = jObj["uuid"].get<uint64_t>();

        // 1. Create or fetch game object
        GameObject gameObject = m_Context.Instantiate(uuid);

        // 2. Deserialize manual components
        if (jObj["components"].contains("scene component"))
        {
            const json& sceneCmp = jObj["components"]["scene component"];

            SceneComponent& scene = gameObject.GetOrAddComponent<SceneComponent>();

            scene.m_Parent = (GameObjectID)sceneCmp["parent"].get<uint64_t>();
            for (auto& childUUID : sceneCmp["children"])
                scene.m_Children.push_back((GameObjectID)childUUID.get<uint64_t>());
        }

        // 3. Deserialize reflected components
        for (auto& [clsName, jComponent] : jObj["components"].items())
        {

            // Look up reflected component class
            const BClass* cls = BClassRegistry::Get().Find(jComponent["id"].get<uint32_t>());
            if (!cls)
            {
                //BOON_WARN("Unknown component class '{}' while deserializing.", clsName);
                continue;
            }

            // Ensure the GameObject has the component
            void* component = gameObject.GetOrAddComponentByClass(cls);

            uint8_t* base = reinterpret_cast<uint8_t*>(component);

            // 4. Deserialize properties
            cls->ForEachProperty([&](const BProperty& prop)
                {
                    if (!jComponent.contains(prop.name))
                        return;

                    const json& jValue = jComponent[prop.name];

                    switch (prop.typeId)
                    {
                    case BTypeId::Int:
                        *reinterpret_cast<int32_t*>(base + prop.offset) = jValue.get<int32_t>();
                        break;

                    case BTypeId::Uint:
                        *reinterpret_cast<uint32_t*>(base + prop.offset) = jValue.get<uint32_t>();
                        break;

                    case BTypeId::Int64:
                        *reinterpret_cast<int64_t*>(base + prop.offset) = jValue.get<int64_t>();
                        break;

                    case BTypeId::Uint64:
                        *reinterpret_cast<uint64_t*>(base + prop.offset) = jValue.get<uint64_t>();
                        break;

                    case BTypeId::Float:
                        *reinterpret_cast<float*>(base + prop.offset) = jValue.get<float>();
                        break;

                    case BTypeId::Bool:
                        *reinterpret_cast<bool*>(base + prop.offset) = jValue.get<bool>();
                        break;

                    case BTypeId::String:
                        *reinterpret_cast<std::string*>(base + prop.offset) = jValue.get<std::string>();
                        break;

                    case BTypeId::Float2:
                    {
                        glm::vec2 v;
                        v.x = jValue[0].get<float>();
                        v.y = jValue[1].get<float>();
                        *reinterpret_cast<glm::vec2*>(base + prop.offset) = v;
                        break;
                    }

                    case BTypeId::Float3:
                    {
                        glm::vec3 v;
                        v.x = jValue[0].get<float>();
                        v.y = jValue[1].get<float>();
                        v.z = jValue[2].get<float>();
                        *reinterpret_cast<glm::vec3*>(base + prop.offset) = v;
                        break;
                    }

                    case BTypeId::Float4:
                    {
                        glm::vec4 v;
                        v.x = jValue[0].get<float>();
                        v.y = jValue[1].get<float>();
                        v.z = jValue[2].get<float>();
                        v.w = jValue[3].get<float>();
                        *reinterpret_cast<glm::vec4*>(base + prop.offset) = v;
                        break;
                    }

                    case BTypeId::Int2:
                    {
                        glm::ivec2 v;
                        v.x = jValue[0].get<int>();
                        v.y = jValue[1].get<int>();
                        *reinterpret_cast<glm::ivec2*>(base + prop.offset) = v;
                        break;
                    }

                    case BTypeId::Int3:
                    {
                        glm::ivec3 v;
                        v.x = jValue[0].get<int>();
                        v.y = jValue[1].get<int>();
                        v.z = jValue[2].get<int>();
                        *reinterpret_cast<glm::ivec3*>(base + prop.offset) = v;
                        break;
                    }

                    case BTypeId::Int4:
                    {
                        glm::ivec4 v;
                        v.x = jValue[0].get<int>();
                        v.y = jValue[1].get<int>();
                        v.z = jValue[2].get<int>();
                        v.w = jValue[3].get<int>();
                        *reinterpret_cast<glm::ivec4*>(base + prop.offset) = v;
                        break;
                    }

                    case BTypeId::AssetRef:
                    {
                        *reinterpret_cast<AssetHandle*>(base + prop.offset) = jValue.get<uint64_t>();
                        break;
                    }

                    default:
                        break;
                    }
                });
        }
    }
}


void Boon::SceneSerializer::Clear()
{
	m_Context.GetRegistry().clear();
    m_Context.m_EntityMap.clear();
}

void Boon::SceneSerializer::Copy(Scene& from)
{
    for (auto pair : from.m_EntityMap)
    {
        GameObject src = GameObject(pair.second, &from);
        GameObject dst = m_Context.Instantiate(pair.first, pair.second);

        BClassRegistry::Get().ForEach([this, &src, &dst](BClass& cls)
            {
                if (src.HasComponentByClass(&cls))
                {
                    void* srcComp = src.GetComponentByClass(&cls);
                    void* dstComp = dst.HasComponentByClass(&cls) 
                        ? dst.GetComponentByClass(&cls) 
                        : dst.AddComponentFromClass(&cls);
                    cls.copyInstance(srcComp, dstComp);
                }
            });

        dst.GetComponent<NameComponent>().Name = src.GetComponent<NameComponent>().Name;
        dst.GetComponent<SceneComponent>().SetScene(&m_Context);
        dst.GetTransform().m_Owner = &dst.GetComponent<SceneComponent>();
    }
}
