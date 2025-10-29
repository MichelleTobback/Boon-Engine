#pragma once
#include "Core/UUID.h"

namespace Boon
{
	typedef UUID AssetHandle;
	typedef size_t AssetTypeID;
	class Asset
	{
	public:
		Asset() : m_Handle() {}
		Asset(AssetHandle handle) : m_Handle(handle) {}
		virtual ~Asset() = default;

		inline AssetHandle GetHandle() const { return m_Handle; }
		inline bool IsValid() const { return m_Handle.IsValid(); }
		inline bool operator()() const { return m_Handle.IsValid(); }

	protected:
		inline void SetHandle(AssetHandle handle) { m_Handle = handle; }

	private:
		AssetHandle m_Handle;
	};
}