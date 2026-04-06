#pragma once
#include <cstdint>
#include <functional>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

namespace Boon
{
    template<typename Signature>
    class Delegate;

    template<typename Ret, typename... Args>
    class Delegate<Ret(Args...)>
    {
    public:
        /**
         * @brief A fast delegate container for callables matching the specified signature.
         *
         * This class stores callable objects and allows binding/unbinding via handles.
         * See member functions for concrete operations and semantics.
         */
        using FunctionType = std::function<Ret(Args...)>;
        using IndexType = std::uint32_t;
        using GenerationType = std::uint32_t;

        struct Handle
        {
            static constexpr IndexType InvalidIndex = std::numeric_limits<IndexType>::max();

            IndexType slot = InvalidIndex;
            GenerationType generation = 0;

            /**
             * @brief Check whether the handle refers to a slot (not explicitly invalid).
             *
             * This checks only that the handle's slot value is not the sentinel invalid
             * index. It does not validate generation or whether the slot is currently bound.
             */
            bool IsValid() const noexcept
            {
                return slot != InvalidIndex;
            }

            friend bool operator==(const Handle& lhs, const Handle& rhs) noexcept
            {
                return lhs.slot == rhs.slot && lhs.generation == rhs.generation;
            }

            friend bool operator!=(const Handle& lhs, const Handle& rhs) noexcept
            {
                return !(lhs == rhs);
            }
        };

    private:
        struct Slot
        {
            FunctionType function{};
            GenerationType generation = 1;
            IndexType denseIndex = 0;
            IndexType nextFree = Handle::InvalidIndex;
            bool active = false;
        };

    public:
        Delegate() = default;
        Delegate(const Delegate&) = default;
        Delegate(Delegate&&) noexcept = default;
        Delegate& operator=(const Delegate&) = default;
        Delegate& operator=(Delegate&&) noexcept = default;
        ~Delegate() = default;

        /**
         * @brief Bind a callable into the delegate.
         *
         * The callable must be convertible to the delegate's FunctionType. A handle
         * is returned which can later be used to unbind the callable.
         *
         * @tparam F Callable type.
         * @param func Callable to bind.
         * @return Handle referring to the bound callable.
         */
        template<typename F>
        Handle Bind(F&& func)
        {
            static_assert(
                std::is_constructible<FunctionType, F&&>::value,
                "Callable cannot be converted to Delegate function type."
                );

            FunctionType stored(std::forward<F>(func));

            IndexType slotIndex;
            if (m_FreeHead != Handle::InvalidIndex)
            {
                slotIndex = m_FreeHead;
                Slot& slot = m_Slots[slotIndex];
                m_FreeHead = slot.nextFree;

                slot.function = std::move(stored);
                slot.active = true;
                slot.nextFree = Handle::InvalidIndex;
                slot.denseIndex = static_cast<IndexType>(m_ActiveSlots.size());

                m_ActiveSlots.push_back(slotIndex);
                return Handle{ slotIndex, slot.generation };
            }

            slotIndex = static_cast<IndexType>(m_Slots.size());
            Slot slot;
            slot.function = std::move(stored);
            slot.generation = 1;
            slot.denseIndex = static_cast<IndexType>(m_ActiveSlots.size());
            slot.nextFree = Handle::InvalidIndex;
            slot.active = true;

            m_Slots.push_back(std::move(slot));
            m_ActiveSlots.push_back(slotIndex);

            return Handle{ slotIndex, 1 };
        }

        /**
         * @brief Convenience; bind a std::function and return its handle.
         */
        Handle operator+=(const FunctionType& func)
        {
            return Bind(func);
        }

        /**
         * @brief Unbind a previously bound callable identified by handle.
         *
         * If the handle is not currently bound this is a no-op.
         *
         * @param handle Handle returned by a prior Bind call.
         */
        void Unbind(Handle handle) noexcept
        {
            if (!IsBound(handle))
                return;

            Slot& slot = m_Slots[handle.slot];

            const IndexType removedDenseIndex = slot.denseIndex;
            const IndexType lastDenseIndex = static_cast<IndexType>(m_ActiveSlots.size() - 1);
            const IndexType movedSlotIndex = m_ActiveSlots[lastDenseIndex];

            if (removedDenseIndex != lastDenseIndex)
            {
                m_ActiveSlots[removedDenseIndex] = movedSlotIndex;
                m_Slots[movedSlotIndex].denseIndex = removedDenseIndex;
            }

            m_ActiveSlots.pop_back();

            slot.function = {};
            slot.active = false;
            ++slot.generation;
            slot.nextFree = m_FreeHead;
            m_FreeHead = handle.slot;
        }

        /**
         * @brief Convenience operator to unbind a handle.
         */
        void operator-=(Handle handle) noexcept
        {
            Unbind(handle);
        }

        /**
         * @brief Check whether a handle currently refers to an active binding.
         *
         * This verifies the handle is valid, within range and that the stored
         * generation and active flag match.
         *
         * @param handle Handle to validate.
         * @return true if the handle refers to an active bound callable, false otherwise.
         */
        bool IsBound(Handle handle) const noexcept
        {
            if (!handle.IsValid())
                return false;

            if (handle.slot >= m_Slots.size())
                return false;

            const Slot& slot = m_Slots[handle.slot];
            return slot.active && slot.generation == handle.generation;
        }

        /**
         * @brief Remove all bindings and reset internal storage.
         */
        void Clear() noexcept
        {
            m_Slots.clear();
            m_ActiveSlots.clear();
            m_FreeHead = Handle::InvalidIndex;
        }

        /**
         * @brief Reserve internal capacity for the given number of bindings.
         *
         * @param capacity Number of bindings to reserve space for.
         */
        void Reserve(std::size_t capacity)
        {
            m_Slots.reserve(capacity);
            m_ActiveSlots.reserve(capacity);
        }

        /**
         * @brief Check whether there are no active bindings.
         *
         * @return true if no functions are bound, false otherwise.
         */
        bool Empty() const noexcept
        {
            return m_ActiveSlots.empty();
        }

        /**
         * @brief Get the number of active bindings.
         *
         * @return Number of currently bound functions.
         */
        std::size_t Size() const noexcept
        {
            return m_ActiveSlots.size();
        }

        /**
         * @brief Get the current capacity of the internal slot storage.
         *
         * @return Capacity of the internal slot vector.
         */
        std::size_t Capacity() const noexcept
        {
            return m_Slots.capacity();
        }

        /**
         * @brief Invoke all bound callables with the provided arguments.
         *
         * Each active bound function is called in the order of the internal
         * active slot list. See implementation for ordering guarantees.
         *
         * @param args Arguments forwarded to bound callables.
         */
        void Invoke(Args... args) const
        {
            const IndexType* active = m_ActiveSlots.data();
            const std::size_t count = m_ActiveSlots.size();

            for (std::size_t i = 0; i < count; ++i)
            {
                const Slot& slot = m_Slots[active[i]];
                slot.function(args...);
            }
        }

        /**
         * @brief Invoke operator forwarding to Invoke.
         */
        void operator()(Args... args) const
        {
            Invoke(args...);
        }

    private:
        std::vector<Slot> m_Slots;
        std::vector<IndexType> m_ActiveSlots;
        IndexType m_FreeHead = Handle::InvalidIndex;
    };
}