#pragma once

#include "MediaFrame.h"


enum AttributeType {

	ATTRIBUTE_TYPE_VIDEO = 0,
	ATTRIBUTE_TYPE_AUDIO,
};


class Sink
{

public:
    virtual int SetSourceAttribute(void* attribute, AttributeType type) = 0;
    virtual int SendFrame(MediaFrame * frame) = 0;
};

