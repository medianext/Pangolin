#pragma once

#include "Sink.h"

class Render :
    public Sink
{

public:
    Render();
    ~Render();

public:
    int SendFrame(MediaFrame * frame, FrameType type);
};

