// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __ENGINE_H__MG__
#define __ENGINE_H__MG__

#include "mgSystem.h"
#include "bss-util\cHighPrecisionTimer.h"

namespace magnesium {
  class MG_DLLEXPORT mgEngine : public mgSystemManager, public bss_util::cHighPrecisionTimer
  {
  public:
    mgEngine();
    ~mgEngine();
    void UpdateDelta();
    inline double GetTimewarp() const { return _timewarp; }
    inline void SetTimewarp(double timewarp) { _timewarp = timewarp; }

    static mgEngine* Instance();

  protected:
    static mgEngine* _instance;
    double _timewarp;
  };
}

#endif