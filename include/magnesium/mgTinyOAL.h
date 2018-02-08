// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __TINYOAL_H__MG__
#define __TINYOAL_H__MG__

#include "mgSystem.h"
#include "tinyoal/TinyOAL.h"

namespace magnesium {
  class MG_DLLEXPORT TinyOALSystem : public tinyoal::TinyOAL, public mgSystem<void>
  {
  public:
    TinyOALSystem(unsigned char defnumbuf = 4, const char* forceOAL = 0, const char* forceOGG = 0, const char* forceFLAC = 0, const char* forceMP3 = 0);
    ~TinyOALSystem();
    virtual void Process() override;
    virtual const char* GetName() const override { return "TinyOAL"; }

    static int Log(const char*, unsigned int, unsigned char, const char*, va_list);
  };
}

#endif