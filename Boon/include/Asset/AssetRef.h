#pragma once

#include "Asset/Asset.h"
#include "Asset/AssetTraits.h"
#include "Core/UUID.h"

#include <functional>
#include <utility>

namespace Boon
{
    class AssetRefResolver
    {
    public:
        using ResolveFn = std::function<Asset*(AssetHandle, AssetType)>;

        static void Bind(ResolveFn fn)
        {
            s_Resolve = std::move(fn);
        }

        static void Unbind()
        {
            s_Resolve = nullptr;
        }

        template<typename T>
        static T* Resolve(AssetHandle handle)
        {
            if (!s_Resolve || !handle.IsValid())
                return nullptr;

            Asset* asset = s_Resolve(handle, AssetTraits<T>::Type);
            return static_cast<T*>(asset);
        }

    private:
        inline static ResolveFn s_Resolve = nullptr;
    };

    template<typename T>
    class AssetRef
    {
    public:
        AssetRef() = default;

        explicit AssetRef(AssetHandle handle)
            : m_Handle(handle)
        {
        }

        T* Get() const
        {
            return AssetRefResolver::Resolve<T>(m_Handle);
        }

        T* operator->() const { return Get(); }
        T& operator*() const { return *Get(); }

        auto Instance() const -> decltype(std::declval<T*>()->GetInstance())
        {
            using ReturnType = decltype(std::declval<T*>()->GetInstance());

            T* asset = Get();
            if (!asset)
                return ReturnType{};

            return asset->GetInstance();
        }

        AssetHandle Handle() const
        {
            return m_Handle;
        }

        bool IsValid() const
        {
            return m_Handle.IsValid();
        }

        explicit operator bool() const
        {
            return IsValid();
        }

        operator AssetHandle() const
        {
            return m_Handle;
        }

        AssetRef& operator=(AssetHandle handle)
        {
            m_Handle = handle;
            return *this;
        }

    private:
        AssetHandle m_Handle = UUID::Null;
    };
}
