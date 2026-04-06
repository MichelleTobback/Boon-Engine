#pragma once
#include "Asset/Asset.h"

namespace Boon
{
    template<typename T>
    class AssetRef 
    {
    public:
        AssetRef() = default;
        AssetRef(AssetHandle h) : m_Handle(h) {}

        AssetRef<T>& operator=(const AssetRef<T>& other)
        {
            if (this == &other)
                return *this;

            m_Handle = other.m_Handle;
            return *this;
        }

        /**
         * @brief Resolve the asset handle to a pointer to the asset.
         *
         * @return Pointer to the asset corresponding to the stored handle, or nullptr if not found.
         */
        T* Get() const;

        T* operator->() const { return Get(); }
        T& operator*() const { return *Get(); }

        /**
         * @brief Obtain the runtime instance held by the referenced asset.
         *
         * @return Shared pointer to the asset instance, or nullptr if the asset is not available.
         */
        auto Instance() const -> std::shared_ptr<typename T::Type>
        {
            T* asset = Get();
            return asset ? asset->GetInstance() : nullptr;
        }

        /**
         * @brief Get the underlying asset handle.
         *
         * @return The AssetHandle stored by this reference.
         */
        AssetHandle Handle() const { return m_Handle; }

        /**
         * @brief Check whether this reference contains a non-zero handle.
         *
         * @return true if the handle is non-zero, false otherwise.
         */
        bool IsValid() const { return m_Handle != 0; }

        operator AssetHandle() const { return m_Handle; }

    private:
        AssetHandle m_Handle = 0;
    };
}
