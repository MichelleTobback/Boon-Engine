#pragma once
#include "Serialization/BinarySerializer.h"
#include "Scene/GameObject.h"

namespace Boon
{
    class IRepSerializer
    {
    public:
        virtual bool IsDirty(GameObject obj) = 0;
        virtual void Serialize(BinarySerializer& ser, GameObject obj) = 0;
        virtual void Deserialize(BinarySerializer& ser, GameObject obj) = 0;
    };
}