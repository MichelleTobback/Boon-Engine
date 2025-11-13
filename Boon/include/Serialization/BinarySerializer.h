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
        enum class Mode
        {
            Writing,
            Reading
        };

        // -------- Constructors --------
        BinarySerializer()
            : m_Mode(Mode::Writing)
        {}

        // Load from raw data for reading
        BinarySerializer(const uint8_t* data, size_t size)
            : m_Mode(Mode::Reading), m_Buffer(data, size), m_ReadPos(0)
        {}

        // Load from existing buffer
        BinarySerializer(const Buffer& buffer)
            : m_Mode(Mode::Reading), m_Buffer(buffer), m_ReadPos(0)
        {}

        // -------- Writing API --------
        template<typename T>
        void Write(const T& value)
        {
            EnsureWriting();
            m_Buffer.Write(value);
        }

        void WriteString(const std::string& str)
        {
            EnsureWriting();
            uint32_t len = (uint32_t)str.size();
            Write(len);
            m_Buffer.WriteRaw(str.data(), len);
        }

        void WriteBytes(const void* data, size_t size)
        {
            EnsureWriting();
            m_Buffer.WriteRaw(data, size);
        }

        // -------- Reading API --------
        template<typename T>
        T Read()
        {
            EnsureReading();
            assert(m_ReadPos + sizeof(T) <= m_Buffer.Size());

            T value{};
            std::memcpy(&value, m_Buffer.Data() + m_ReadPos, sizeof(T));
            m_ReadPos += sizeof(T);
            return value;
        }

        std::string ReadString()
        {
            EnsureReading();
            uint32_t len = Read<uint32_t>();
            assert(m_ReadPos + len <= m_Buffer.Size());

            std::string str(len, '\0');
            std::memcpy(str.data(), m_Buffer.Data() + m_ReadPos, len);
            m_ReadPos += len;
            return str;
        }

        void ReadBytes(void* out, size_t size)
        {
            EnsureReading();
            assert(m_ReadPos + size <= m_Buffer.Size());
            std::memcpy(out, m_Buffer.Data() + m_ReadPos, size);
            m_ReadPos += size;
        }

        bool HasRemaining() const
        {
            return m_ReadPos < m_Buffer.Size();
        }

        const uint8_t* Data() const { return m_Buffer.Data(); }
        size_t Size() const { return m_Buffer.Size(); }

        Buffer& GetBuffer() { return m_Buffer; }
        const Buffer& GetBuffer() const { return m_Buffer; }

        Mode GetMode() const { return m_Mode; }

    private:
        Mode m_Mode = Mode::Writing;
        Buffer m_Buffer;
        size_t m_ReadPos = 0;

        void EnsureWriting() const
        {
            assert(m_Mode == Mode::Writing && "Attempted to write in reading mode!");
        }

        void EnsureReading() const
        {
            assert(m_Mode == Mode::Reading && "Attempted to read in writing mode!");
        }
    };
}
