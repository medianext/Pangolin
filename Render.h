#pragma once

#include "Sink.h"

class Render :
    public Sink
{

public:
    static void Init();
    static void Uninit();
    static Render* GetRender();
    ~Render();
private:
    Render();

public:
    int SendFrame(MediaFrame * frame);
};

