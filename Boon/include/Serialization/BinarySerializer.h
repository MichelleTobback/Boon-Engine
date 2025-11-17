#pragma once
#include "Core/Memory/Buffer.h"
#include <string>
#include <cassert>
#include <cstring>

namespace Boon
{
    class BinarySerializer
    {
    public:
        enum class Mode { Writing, Reading };

        // ---------------- Constructors ----------------
        BinarySerializer()
            : m_Mode(Mode::Writing)
        {
        }

        BinarySerializer(const uint8_t* data, size_t size)
            : m_Mode(Mode::Reading),
            m_Buffer(data, size),
            m_ReadBitPos(0)
        {
        }

        BinarySerializer(const Buffer& buffer)
            : m_Mode(Mode::Reading),
            m_Buffer(buffer),
            m_ReadBitPos(0)
        {
        }

        // =====================================================
        //                   BITPACK WRITING
        // =====================================================
        inline void WriteBits(uint32_t value, int bitCount)
        {
            EnsureWriting();

            for (int i = 0; i < bitCount; i++)
            {
                uint32_t bit = (value >> i) & 1;

                // Ensure the target byte exists
                if ((m_WriteBitPos >> 3) >= m_Buffer.Size())
                    m_Buffer.Vector().push_back(0);

                uint8_t& byte = m_Buffer.Vector()[m_WriteBitPos >> 3];
                byte |= (bit << (m_WriteBitPos & 7));

                m_WriteBitPos++;
            }
        }

        inline void AlignWrite()
        {
            while (m_WriteBitPos & 7)
                WriteBits(0, 1);
        }

        inline void WriteBytes(const void* data, size_t size)
        {
            EnsureWriting();
            AlignWrite();

            size_t bytePos = m_WriteBitPos >> 3;
            size_t needed = bytePos + size;

            if (needed > m_Buffer.Size())
                m_Buffer.Vector().resize(needed);

            memcpy(m_Buffer.Data() + bytePos, data, size);
            m_WriteBitPos += size * 8;
        }

        template<typename T>
        inline void Write(const T& value)
        {
            WriteBytes(&value, sizeof(T));
        }

        inline void WriteString(const std::string& s)
        {
            uint32_t len = (uint32_t)s.size();
            Write(len);
            WriteBytes(s.data(), len);
        }

        // =====================================================
        //                   BITPACK READING
        // =====================================================
        inline uint32_t ReadBits(int bitCount)
        {
            EnsureReading();

            uint32_t v = 0;
            for (int i = 0; i < bitCount; i++)
            {
                assert((m_ReadBitPos >> 3) < m_Buffer.Size());
                uint8_t byte = m_Buffer.Data()[m_ReadBitPos >> 3];
                uint32_t bit = (byte >> (m_ReadBitPos & 7)) & 1;
                v |= (bit << i);
                m_ReadBitPos++;
            }
            return v;
        }

        inline void AlignRead()
        {
            while (m_ReadBitPos & 7)
                ReadBits(1);
        }

        inline void ReadBytes(void* out, size_t size)
        {
            EnsureReading();
            AlignRead();

            size_t bytePos = m_ReadBitPos >> 3;
            assert(bytePos + size <= m_Buffer.Size());

            memcpy(out, m_Buffer.Data() + bytePos, size);
            m_ReadBitPos += size * 8;
        }

        template<typename T>
        inline T Read()
        {
            T v{};
            ReadBytes(&v, sizeof(T));
            return v;
        }

        inline std::string ReadString()
        {
            uint32_t len = Read<uint32_t>();
            std::string s(len, 0);
            ReadBytes(s.data(), len);
            return s;
        }

        // =====================================================
        // Utilities
        // =====================================================
        inline const uint8_t* Data() const { return m_Buffer.Data(); }
        inline size_t Size() const { return m_Buffer.Size(); }
        inline bool HasRemaining() const { return (m_ReadBitPos >> 3) < m_Buffer.Size(); }

        inline size_t GetWriteBitPos() const { return m_WriteBitPos; }
        inline size_t GetReadBitPos()  const { return m_ReadBitPos; }

        inline Buffer& GetBuffer() { return m_Buffer; }
        inline const Buffer& GetBuffer() const { return m_Buffer; }

        inline Mode GetMode() const { return m_Mode; }

    private:
        Mode m_Mode;
        Buffer m_Buffer;
        size_t m_WriteBitPos = 0;
        size_t m_ReadBitPos = 0;

        inline void EnsureWriting() const
        {
            assert(m_Mode == Mode::Writing && "Write in read mode!");
        }

        inline void EnsureReading() const
        {
            assert(m_Mode == Mode::Reading && "Read in write mode!");
        }
    };
}
