#pragma once
#include <cstdint>
#include <cstddef>

namespace Boon
{
    class BufferView
    {
    public:
        BufferView()
            : m_Data(nullptr), m_Size(0) {}

        BufferView(const void* data, size_t size)
            : m_Data((const uint8_t*)data), m_Size(size)
        {}

        const uint8_t* Data() const { return m_Data; }
        size_t Size() const { return m_Size; }

        bool Empty() const { return m_Size == 0; }

    private:
        const uint8_t* m_Data;
        size_t m_Size;
    };
}
