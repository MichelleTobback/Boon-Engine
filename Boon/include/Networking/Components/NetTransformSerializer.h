#pragma once
#include "Networking/IRepSerializer.h"
#include "Networking/Components/NetTransform.h"

namespace Boon
{
    using namespace ReplicationUtils;

    class NetTransformSerializer : public IRepSerializer
    {
    public:
        virtual bool IsDirty(GameObject obj) override
        {
            auto& t = obj.GetComponent<NetTransform>();

            return t.DirtyMask != 0;
        }

        virtual void Serialize(BinarySerializer& ser, GameObject obj) override
        {
            auto& t = obj.GetComponent<NetTransform>();

            ser.Write<uint8_t>(t.DirtyMask);

            if (t.DirtyMask & (uint32_t)NetTransform::DirtyFlags::PosX)
                ser.WriteBits(t.QPosX, 16);

            if (t.DirtyMask & (uint32_t)NetTransform::DirtyFlags::PosY)
                ser.WriteBits(t.QPosY, 16);

            if (t.DirtyMask & (uint32_t)NetTransform::DirtyFlags::PosZ)
                ser.WriteBits(t.QPosZ, 16);

            if (t.DirtyMask & (uint32_t)NetTransform::DirtyFlags::Rot)
                ser.WriteBits(t.QRotDeg, 16);

            if (t.DirtyMask & (uint32_t)NetTransform::DirtyFlags::ScaleX)
                ser.WriteBits(t.QScaleX, 16);

            if (t.DirtyMask & (uint32_t)NetTransform::DirtyFlags::ScaleY)
                ser.WriteBits(t.QScaleY, 16);

            t.LastQPosX = t.QPosX;
            t.LastQPosY = t.QPosY;
            t.LastQPosZ = t.QPosZ;
            t.LastQRotDeg = t.QRotDeg;
            t.LastQScaleX = t.QScaleX;
            t.LastQScaleY = t.QScaleY;
        }

        virtual void Deserialize(BinarySerializer& ser, GameObject obj) override
        {
            auto& t = obj.GetComponent<NetTransform>();

            t.DirtyMask = (uint32_t)ser.Read<uint8_t>();

            if (t.DirtyMask & (uint32_t)NetTransform::DirtyFlags::PosX)
                t.QPosX = (int16_t)ser.ReadBits(16);

            if (t.DirtyMask & (uint32_t)NetTransform::DirtyFlags::PosY)
                t.QPosY = (int16_t)ser.ReadBits(16);

            if (t.DirtyMask & (uint32_t)NetTransform::DirtyFlags::PosZ)
                t.QPosZ = (int16_t)ser.ReadBits(16);

            if (t.DirtyMask & (uint32_t)NetTransform::DirtyFlags::Rot)
                t.QRotDeg = (uint16_t)ser.ReadBits(16);

            if (t.DirtyMask & (uint32_t)NetTransform::DirtyFlags::ScaleX)
                t.QScaleX = (int16_t)ser.ReadBits(16);

            if (t.DirtyMask & (uint32_t)NetTransform::DirtyFlags::ScaleY)
                t.QScaleY = (int16_t)ser.ReadBits(16);
        }
    };
}