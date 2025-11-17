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
        ~ReplicatedClass()
        {
            //if (serializer)
            //    delete serializer;
        }

        const BClass* cls;
        std::shared_ptr<IRepSerializer> serializer = nullptr;

        size_t size = 0;

        // Static: flag -> reflected replicated fields
        std::vector<std::unordered_map<uint32_t, ReplicatedField>> fields;

        const ReplicatedField& GetField(uint32_t index, uint16_t set = 0)
        {
            return fields[set][index];
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

            // A "property set" contains up to 31 properties → 31 bits of dirtyFlags
            std::unordered_map<uint32_t, ReplicatedField> currentSet;
            currentSet.reserve(31);

            uint32_t nextFlagBit = 1;

            cls->ForEachProperty([&](const BProperty& prop)
                {
                    if (!prop.HasMeta("Replicated"))
                        return;

                    // Start a new set if current one is full
                    if (currentSet.size() >= 31)
                    {
                        obj.fields.push_back(std::move(currentSet));
                        currentSet = {};
                        currentSet.reserve(31);
                        nextFlagBit = 1;
                    }

                    // Build replicated field descriptor
                    ReplicatedField rf{};
                    rf.pProp = &prop;
                    rf.Packedoffset = (uint32_t)obj.size;

                    // Insert into current set
                    currentSet[nextFlagBit] = rf;

                    // Advance running offset (snapshot layout)
                    obj.size += prop.size;

                    nextFlagBit <<= 1; // next bit flag
                });

            // Push final set if any
            if (!currentSet.empty())
                obj.fields.push_back(std::move(currentSet));

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