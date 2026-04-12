#pragma once
#include <string>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <optional>
#include <utility>

namespace Boon 
{

    enum class BTypeId : uint8_t
    {
        Unknown,
        Int,
        Int64,
        Uint,
        Uint64,
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
        BRef,
        Enum,
        UserDefined
    };

    struct BPropertyMeta
    {
        std::string key;
        std::string value;
    };

    /**
     * @brief Reflection information for a single class property/member.
     *
     * Contains the property's name, type name, byte offset within the
     * instance, size and optional metadata entries.
     */
    struct BProperty
    {
        std::string name;
        std::string typeName;
        std::size_t offset{};
        std::size_t size{};
        BTypeId typeId{ BTypeId::Unknown };
        std::vector<BPropertyMeta> meta;

        /**
         * @brief Check whether metadata with the given key exists for this property.
         *
         * @param key Metadata key to search for.
         * @return true if a metadata entry with the key exists, false otherwise.
         */
        bool HasMeta(const std::string& key) const
        {
            for (auto& m : meta)
                if (m.key == key)
                    return true;
            return false;
        }

        /**
         * @brief Get the value for a metadata entry if present.
         *
         * @param key Metadata key to retrieve.
         * @return Optional string containing the metadata value when present.
         */
        std::optional<std::string> GetMeta(const std::string& key) const
        {
            for (auto& m : meta)
                if (m.key == key)
                    return m.value;
            return std::nullopt;
        }

        bool IsVariant() const;
    };

} // namespace Boon
