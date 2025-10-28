#include "Core/UUID.h"

#include "Core/Random.h"

#include <unordered_map>
#include <string>

Boon::UUID::UUID()
	: m_Uuid{}
{
	m_Uuid = Random::GetRandomUint64();
}

Boon::UUID::UUID(uint64_t uuid)
	: m_Uuid{uuid}
{
}

bool Boon::UUID::IsValid() const
{
	return m_Uuid != 0u;
}
