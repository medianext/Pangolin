#include "stdafx.h"
#include "Render.h"


static Render* instanse;

Render::Render()
{
}


Render::~Render()
{
}


void Render::Init()
{
    instanse = new Render();
}


void Render::Uninit()
{
    if (instanse)
    {
        delete instanse;
    }
}


Render* Render::GetRender()
{
    return instanse;
}


int Render::SendFrame(MediaFrame * frame, FrameType type)
{
    return 0;
}
