#pragma once
#include "Networking/IRepSerializer.h"
#include "Networking/Components/NetRigidbody2D.h"

namespace Boon
{
    using namespace ReplicationUtils;

    class NetRigidbody2DSerializer : public IRepSerializer
    {
    public:
        virtual bool IsDirty(GameObject obj) override
        {
            auto& rb = obj.GetComponent<NetRigidbody2D>();

            return rb.DirtyMask != 0;
        }
        
        virtual void Serialize(BinarySerializer& ser, GameObject obj) override
        {
            auto& rb = obj.GetComponent<NetRigidbody2D>();

            ser.Write<uint8_t>(rb.DirtyMask);

            if (rb.DirtyMask & (uint32_t)NetRigidbody2D::DirtyFlags::PosX)
                ser.WriteBits(rb.QPosX, 16);

            if (rb.DirtyMask & (uint32_t)NetRigidbody2D::DirtyFlags::PosY)
                ser.WriteBits(rb.QPosY, 16);

            if (rb.DirtyMask & (uint32_t)NetRigidbody2D::DirtyFlags::PosZ)
                ser.WriteBits(rb.QPosZ, 16);

            if (rb.DirtyMask & (uint32_t)NetRigidbody2D::DirtyFlags::Rot)
                ser.WriteBits(rb.QRotDeg, 16);

            rb.LastQPosX = rb.QPosX;
            rb.LastQPosY = rb.QPosY;
            rb.LastQPosZ = rb.QPosZ;
            rb.LastQRotDeg = rb.QRotDeg;
        }

        virtual void Deserialize(BinarySerializer& ser, GameObject obj) override
        {
            auto& rb = obj.GetComponent<NetRigidbody2D>();

            rb.DirtyMask = (uint32_t)ser.Read<uint8_t>();

            if (rb.DirtyMask & (uint32_t)NetRigidbody2D::DirtyFlags::PosX)
                rb.QPosX = (int16_t)ser.ReadBits(16);

            if (rb.DirtyMask & (uint32_t)NetRigidbody2D::DirtyFlags::PosY)
                rb.QPosY = (int16_t)ser.ReadBits(16);

            if (rb.DirtyMask & (uint32_t)NetRigidbody2D::DirtyFlags::PosZ)
                rb.QPosZ = (int16_t)ser.ReadBits(16);

            if (rb.DirtyMask & (uint32_t)NetRigidbody2D::DirtyFlags::Rot)
                rb.QRotDeg = (uint16_t)ser.ReadBits(16);
        }
    };
}