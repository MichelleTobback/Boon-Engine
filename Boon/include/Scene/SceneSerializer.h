#pragma once
#include "Scene.h"
#include <filesystem>

namespace Boon
{
	class SceneSerializer final
	{
	public:
		/**
		 * @brief Construct a serializer for the provided scene.
		 * @param scene Scene instance to serialize/deserialize.
		 */
		SceneSerializer(Scene& scene);

		/**
		 * @brief Serialize the scene to the destination path.
		 * @param dst Destination file path to write the scene representation.
		 */
		void Serialize(const std::filesystem::path& dst);

		/**
		 * @brief Deserialize scene data from the provided source path.
		 * @param src Source file path to read scene data from.
		 */
		void Deserialize(const std::filesystem::path& src);

		/**
		 * @brief Clear serialized data / internal caches used by the serializer.
		 */
		void Clear();

		/**
		 * @brief Copy scene contents from another scene instance.
		 * @param from Scene to copy data from.
		 */
		void Copy(Scene& from);

	private:
		Scene& m_Context;
	};
}