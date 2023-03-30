#include "ActorUID.hpp"

ActorUID const ActorUID::INVALID = ActorUID(0xFFFF, 0xFFFF);

ActorUID::ActorUID()
{
	m_data = 0xFFFFFFFF;
}

ActorUID::ActorUID(int index, int salt)
{
	m_data = (salt << 16) + index;
}

void ActorUID::Invalidate()
{
	m_data = 0xFFFFFFFF;
}

bool ActorUID::IsValid() const
{
	return m_data != 0xFFFFFFFF;
}

int ActorUID::GetIndex() const
{
	return m_data & 0x0000FFFF;
}

bool ActorUID::operator!=(const ActorUID& other) const
{
	return other.m_data != m_data;
}

bool ActorUID::operator==(const ActorUID& other) const
{
	return other.m_data == m_data;
}
