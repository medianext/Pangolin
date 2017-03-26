/*
* Author：王德明
* Email：phight@163.com
* QQ群：220954528
*/

#pragma once

enum PacketType {

	PACKET_TYPE_VIDEO = 0,
	PACKET_TYPE_AUDIO,
};


class MediaPacket
{
public:
	MediaPacket();
	MediaPacket(PacketType type, int dataSize);
	~MediaPacket();

public:
    PacketType m_PacketType;
    int   m_dataSize;
	BYTE  *m_pData;

	bool m_bKeyframe;

	unsigned int m_uTimestamp;
};

