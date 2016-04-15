// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgEngine.h"

using namespace magnesium;

#ifdef BSS_COMPILER_MSC
#if defined(BSS_DEBUG) && defined(BSS_CPU_x86_64)
#pragma comment(lib, "../lib/bss-util_d.lib")
#elif defined(BSS_CPU_x86_64)
#pragma comment(lib, "../lib/bss-util.lib")
#elif defined(BSS_DEBUG)
#pragma comment(lib, "../lib/bss-util32_d.lib")
#else
#pragma comment(lib, "../lib/bss-util32.lib")
#endif
#endif

mgEngine* mgEngine::_instance = 0;

mgEngine::mgEngine() { _instance = this; }
mgEngine::~mgEngine() { _instance = 0; }

mgEngine* mgEngine::Instance() { return _instance; }
void mgEngine::UpdateDelta()
{
  if(_timewarp == 1.0)
    Update();
  else
    Update(_timewarp);
}

