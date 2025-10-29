#pragma once
#include "Asset.h"

#include <string>
#include <vector>
#include <memory>

namespace Boon
{
	class AssetLoader
	{
	public:
		AssetLoader(const std::vector<std::string>& extension)
			: m_Extensions{ extension } {}
		virtual ~AssetLoader() = default;

		inline const std::vector<std::string>& GetExtensions() const { return m_Extensions; }

		virtual std::unique_ptr<Asset> Load(const std::string& path) = 0;

	private:
		std::vector<std::string> m_Extensions;
	};
}