#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <string>

namespace Boon
{
    class Buffer
    {
    public:
        Buffer() = default;

        explicit Buffer(size_t size)
            : m_Data(size)
        {}

        explicit Buffer(const std::vector<uint8_t>& data)
            : m_Data(data)
        {}

        Buffer(const void* data, size_t size)
            : m_Data((uint8_t*)data, (uint8_t*)data + size)
        {}

        // ---------- Write / Append ----------
        void Append(const void* data, size_t size)
        {
            size_t start = m_Data.size();
            m_Data.resize(start + size);
            std::memcpy(m_Data.data() + start, data, size);
        }

        void Append(const Buffer& other)
        {
            size_t start = m_Data.size();
            m_Data.resize(start + other.Size());
            std::memcpy(m_Data.data() + start, other.Data(), other.Size());
        }

        template<typename T>
        void Write(const T& value)
        {
            static_assert(std::is_trivially_copyable<T>::value,
                "Buffer::Write requires trivially copyable type");

            Append(&value, sizeof(T));
        }

        void WriteRaw(const void* data, size_t size)
        {
            Append(data, size);
        }

        void WriteString(const std::string& str)
        {
            uint32_t len = (uint32_t)str.size();
            Write<uint32_t>(len);
            WriteRaw(str.data(), len);
        }

        // ---------- Read ----------
        template<typename T>
        T Read(size_t& offset) const
        {
            static_assert(std::is_trivially_copyable<T>::value,
                "Buffer::Read requires trivially copyable type");

            T val{};
            std::memcpy(&val, m_Data.data() + offset, sizeof(T));
            offset += sizeof(T);
            return val;
        }

        void ReadRaw(void* out, size_t size, size_t& offset) const
        {
            std::memcpy(out, m_Data.data() + offset, size);
            offset += size;
        }

        std::string ReadString(size_t& offset) const
        {
            uint32_t len = Read<uint32_t>(offset);

            std::string str;
            str.resize(len);

            ReadRaw(str.data(), len, offset);
            return str;
        }

        // ---------- Utility ----------
        uint8_t* Data() { return m_Data.data(); }
        const uint8_t* Data() const { return m_Data.data(); }

        uint8_t* DataAt(uint32_t pos) { return m_Data.data() + pos; }
        const uint8_t* DataAt(uint32_t pos) const { return m_Data.data() + pos; }

        size_t Size() const { return m_Data.size(); }
        bool Empty() const { return m_Data.empty(); }

        void Clear() { m_Data.clear(); }
        void Reserve(size_t cap) { m_Data.reserve(cap); }

        std::vector<uint8_t>& Vector() { return m_Data; }
        const std::vector<uint8_t>& Vector() const { return m_Data; }

    private:
        std::vector<uint8_t> m_Data;
    };
}
