#pragma once

#include "MediaFrame.h"

class Sink
{

public:
    virtual int SendFrame(MediaFrame * frame, FrameType type) = 0;
};

