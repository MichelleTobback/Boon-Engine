#pragma once
#include <string>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <optional>
#include <utility>

namespace Boon {

    enum class BTypeId : uint8_t
    {
        Unknown,
        Int,
        Float,
        Bool,
        String,
        Float2,
        Float3,
        Float4,
        Mat4,
        Int2,
        Int3,
        Int4,
        AssetRef,
        UserDefined
    };

    struct BPropertyMeta
    {
        std::string key;
        std::string value;
    };

    struct BProperty
    {
        std::string name;
        std::string typeName;
        std::size_t offset{};
        std::size_t size{};
        BTypeId typeId{ BTypeId::Unknown };
        std::vector<BPropertyMeta> meta;

        bool HasMeta(const std::string& key) const
        {
            for (auto& m : meta)
                if (m.key == key)
                    return true;
            return false;
        }

        std::optional<std::string> GetMeta(const std::string& key) const
        {
            for (auto& m : meta)
                if (m.key == key)
                    return m.value;
            return std::nullopt;
        }
    };

} // namespace Boon
