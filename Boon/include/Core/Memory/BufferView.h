#pragma once
#include <cstdint>
#include <cstddef>

namespace Boon
{
    class BufferView
    {
    public:
        /**
         * @brief Non-owning view over a byte range.
         *
         * Does not manage lifetime of the underlying memory.
         */
        BufferView()
            : m_Data(nullptr), m_Size(0) {}

        /**
         * @brief Construct a view over the provided data pointer and size.
         *
         * @param data Pointer to the memory to view.
         * @param size Size in bytes of the view.
         */
        BufferView(const void* data, size_t size)
            : m_Data((const uint8_t*)data), m_Size(size)
        {}

        /**
         * @brief Pointer to the viewed data.
         */
        const uint8_t* Data() const { return m_Data; }

        /**
         * @brief Size of the view in bytes.
         */
        size_t Size() const { return m_Size; }

        /**
         * @brief Check whether the view is empty.
         */
        bool Empty() const { return m_Size == 0; }

    private:
        const uint8_t* m_Data;
        size_t m_Size;
    };
}
