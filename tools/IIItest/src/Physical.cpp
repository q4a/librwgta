#include "III.h"

CPhysical::CPhysical(void)
{
	m_doCollision = 1;
}

CPhysical::~CPhysical(void)
{
}

void
CPhysical::AddToMovingList(void)
{
	m_movingListNode = CWorld::GetMovingEntityList().InsertItem(this);
}
