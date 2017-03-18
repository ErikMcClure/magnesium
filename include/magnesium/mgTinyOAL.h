// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __TINYOAL_H__MG__
#define __TINYOAL_H__MG__

#include "mgSystem.h"
#include "tinyoal/cTinyOAL.h"

namespace magnesium {
  class MG_DLLEXPORT TinyOALSystem : public tinyoal::cTinyOAL, public mgSystem<void>
  {
  public:
    TinyOALSystem(unsigned char defnumbuf = 4, const char* forceOAL = 0, const char* forceOGG = 0, const char* forceFLAC = 0, const char* forceMP3 = 0);
    ~TinyOALSystem();
    virtual void Process() override;

    static int Log(const char*, unsigned int, unsigned char, const char*, va_list);
  };
}

#endif