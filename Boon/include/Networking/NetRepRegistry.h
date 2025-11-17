#pragma once
#include "Reflection/BClass.h"
#include "Core/Memory/Buffer.h"
#include "Networking/IRepSerializer.h"

#include <memory>
#include <unordered_map>

namespace Boon
{
    struct ReplicatedField
    {
        const BProperty* pProp;
        uint32_t Offset() const { return pProp->offset; }
        uint32_t Size() const { return pProp->size; }

        uint32_t Packedoffset;
    };

    struct ReplicatedClass
    {
        ~ReplicatedClass() {}

        const BClass* cls;
        std::shared_ptr<IRepSerializer> serializer = nullptr;

        size_t size = 0;

        // Replication sets (31 flags per set)
        std::vector<std::unordered_map<uint32_t, ReplicatedField>> fields;

        // -----------------------------
        // RPC SUPPORT
        // -----------------------------
        std::unordered_map<uint32_t, const BFunction*> serverRPCs;
        std::unordered_map<uint32_t, const BFunction*> clientRPCs;

        inline const BFunction* FindServerRPC(uint32_t id) const
        {
            auto it = serverRPCs.find(id);
            return it == serverRPCs.end() ? nullptr : it->second;
        }
        inline const BFunction* FindClientRPC(uint32_t id) const
        {
            auto it = clientRPCs.find(id);
            return it == clientRPCs.end() ? nullptr : it->second;
        }
    };

    struct NetRepRegistry
    {
        static NetRepRegistry& Get()
        {
            static NetRepRegistry instance = NetRepRegistry();
            return instance;
        }

        void Register(BClass* cls, IRepSerializer* pSerializer = nullptr)
        {
            ReplicatedClass obj{};
            obj.cls = cls;
            obj.serializer = std::shared_ptr<IRepSerializer>(std::move(pSerializer));
            obj.size = 0;

            // A "property set" contains up to 31 properties
            std::unordered_map<uint32_t, ReplicatedField> currentSet;
            currentSet.reserve(31);

            uint32_t nextFlagBit = 1;

            // Build replication layout
            cls->ForEachProperty([&](const BProperty& prop)
                {
                    if (!prop.HasMeta("Replicated"))
                        return;

                    if (currentSet.size() >= 31)
                    {
                        obj.fields.push_back(std::move(currentSet));
                        currentSet.clear();
                        currentSet.reserve(31);
                        nextFlagBit = 1;
                    }

                    ReplicatedField rf{};
                    rf.pProp = &prop;
                    rf.Packedoffset = (uint32_t)obj.size;

                    currentSet[nextFlagBit] = rf;

                    obj.size += prop.size;
                    nextFlagBit <<= 1;
                });

            if (!currentSet.empty())
                obj.fields.push_back(std::move(currentSet));

            // ------------------------------------------
            // RPC DETECTION
            // ------------------------------------------
            for (auto& fn : cls->functions)
            {
                for (auto& meta : fn.meta)
                {
                    if (meta.key == "RPC")
                    {
                        if (meta.value == "Server")
                            obj.serverRPCs[fn.id] = &fn;

                        else if (meta.value == "Client")
                            obj.clientRPCs[fn.id] = &fn;
                    }
                }
            }

            // Register
            m_ReplicatedClasses[cls->hash] = std::move(obj);
        }

        ReplicatedClass& GetClass(BClassID id)
        {
            return m_ReplicatedClasses.at(id);
        }

        template<typename Fn>
        void ForEach(Fn&& fn)
        {
            for (auto& [_, cls] : m_ReplicatedClasses)
                fn(cls);
        }

    private:
        std::unordered_map<BClassID, ReplicatedClass> m_ReplicatedClasses;
    };
}
