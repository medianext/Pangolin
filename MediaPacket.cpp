#include "stdafx.h"
#include "MediaPacket.h"


MediaPacket::MediaPacket() :
	m_pData(NULL)
{
}


MediaPacket::MediaPacket(PacketType type, int dataSize)
{
	m_pData = new BYTE[dataSize];
	m_PacketType = type;
}


MediaPacket::~MediaPacket()
{
	if (m_pData!=NULL)
	{
		delete m_pData;
		m_pData = NULL;
	}
}
