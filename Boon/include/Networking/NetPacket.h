#pragma once
#include <cstdint>
#include "Core/Memory/Buffer.h"
#include "Serialization/BinarySerializer.h"

namespace Boon
{
    // -------------------------------------------------------------
    // All packet types used by the networking system
    // -------------------------------------------------------------
    enum class ENetPacketType : uint8_t
    {
        None = 0,

        // Scene / Object Management
        Spawn = 1,
        Despawn = 2,
        Component = 3,
        LoadScene = 4,

        // Data Flow
        Replication = 5,
        RPC = 6,

        // Connection / Handshake
        Handshake = 7,
        Ping = 8,
        Pong = 9
    };

    // -------------------------------------------------------------
    // Standard packet header (16 bytes total)
    // -------------------------------------------------------------
    struct NetPacketHeader
    {
        NetPacketHeader() = default;
        NetPacketHeader(ENetPacketType type)
            : Type(type) {}

        uint16_t ProtocolVersion = 1;
        ENetPacketType Type = ENetPacketType::None;
        uint8_t Reserved0 = 0;    // alignment

        uint32_t PayloadSize = 0;
        uint32_t Flags = 0;       // compressed, reliable, fragmented, etc.
        uint32_t ServerTick = 0;  // for time sync, lagcomp

        static constexpr size_t Size()
        {
            return sizeof(NetPacketHeader);
        }
    };

    // -------------------------------------------------------------
    // NetPacket combines:
    // - header (type + size)
    // - BinarySerializer
    // - Buffer
    // -------------------------------------------------------------
    class NetPacket
    {
    public:
        // Writing new packet
        NetPacket(ENetPacketType type)
            : m_Header{ type }, m_Serializer()
        {}

        // Reading received packet
        NetPacket(const uint8_t* data, size_t size)
        {
            // Header first
            std::memcpy(&m_Header, data, NetPacketHeader::Size());

            // Remaining buffer for serializer
            size_t payloadSize = size - NetPacketHeader::Size();
            m_Serializer = BinarySerializer(data + NetPacketHeader::Size(), payloadSize);
        }

        // ---------------------------------------------------------
        // Write API
        // ---------------------------------------------------------
        template<typename T>
        void Write(const T& value)
        {
            m_Serializer.Write(value);
        }

        void WriteString(const std::string& str)
        {
            m_Serializer.WriteString(str);
        }

        void WriteBytes(const void* data, size_t size)
        {
            m_Serializer.WriteBytes(data, size);
        }

        // ---------------------------------------------------------
        // Read API
        // ---------------------------------------------------------
        template<typename T>
        T Read() { return m_Serializer.Read<T>(); }

        std::string ReadString()
        {
            return m_Serializer.ReadString();
        }

        void ReadBytes(void* out, size_t size)
        {
            m_Serializer.ReadBytes(out, size);
        }

        // ---------------------------------------------------------
        // Finalize packet into raw byte stream
        // ---------------------------------------------------------
        void BuildBuffer(Buffer& output)
        {
            const Buffer& payload = m_Serializer.GetBuffer();

            m_Header.PayloadSize = static_cast<uint32_t>(payload.Size());

            output.Reserve(NetPacketHeader::Size() + payload.Size());

            output.Append(&m_Header, NetPacketHeader::Size());
            output.Append(payload);
        }

        // Direct data for sending
        const uint8_t* RawData()
        {
            if (m_BuiltCache.Empty())
                BuildBuffer(m_BuiltCache);
            return m_BuiltCache.Data();
        }

        size_t RawSize()
        {
            if (m_BuiltCache.Empty())
                BuildBuffer(m_BuiltCache);
            return m_BuiltCache.Size();
        }

        ENetPacketType GetType() const { return m_Header.Type; }

        BinarySerializer& GetSerializer() { return m_Serializer; }
        const BinarySerializer& GetSerializer() const { return m_Serializer; }

    private:
        NetPacketHeader m_Header{};
        BinarySerializer m_Serializer;

        // built packet for sending (only built once)
        Buffer m_BuiltCache;
    };
}
