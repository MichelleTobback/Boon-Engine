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

        T* Get() const;

        T* operator->() const { return Get(); }
        T& operator*() const { return *Get(); }

        auto Instance() const -> std::shared_ptr<typename T::Type>
        {
            T* asset = Get();
            return asset ? asset->GetInstance() : nullptr;
        }

        AssetHandle Handle() const { return m_Handle; }
        bool IsValid() const { return m_Handle != 0; }

        operator AssetHandle() const { return m_Handle; }

    private:
        AssetHandle m_Handle = 0;
    };
}
