#pragma once

#include "MediaFrame.h"

class Sink
{

public:
    virtual int SetAttribute(void* attribute) = 0;
    virtual int SendFrame(MediaFrame * frame) = 0;
};

