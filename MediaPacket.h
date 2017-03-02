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
};

