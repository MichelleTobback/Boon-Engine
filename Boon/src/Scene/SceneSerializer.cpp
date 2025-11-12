#include "Scene/SceneSerializer.h"
#include "Component/NameComponent.h"
#include "Component/SceneComponent.h"

#include "Scene/GameObject.h"
#include "Reflection/BClass.h"

using namespace Boon;

Boon::SceneSerializer::SceneSerializer(Scene& scene)
	: m_Context(scene){}

void Boon::SceneSerializer::Serialize(const std::string& dst)
{
}

void Boon::SceneSerializer::Deserialize(const std::string& src)
{
}

void Boon::SceneSerializer::Clear()
{
	m_Context.GetRegistry().clear();
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
