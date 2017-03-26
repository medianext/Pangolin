/*
* Author：王德明
* Email：phight@163.com
* QQ群：220954528
*/

#include "stdafx.h"
#include "MediaPacket.h"


MediaPacket::MediaPacket() :
	m_pData(NULL),
    m_dataSize(0),
	m_bKeyframe(false),
	m_uTimestamp(0)
{
}


MediaPacket::MediaPacket(PacketType type, int dataSize) :
    m_PacketType(type),
	m_dataSize(dataSize),
	m_bKeyframe(false),
	m_uTimestamp(0)
{
	m_pData = new BYTE[dataSize];
}


MediaPacket::~MediaPacket()
{
	if (m_pData!=NULL)
	{
		delete m_pData;
		m_pData = NULL;
	}
}
