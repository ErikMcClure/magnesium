// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgTinyOAL.h"
#include "mgEngine.h"

using namespace magnesium;
using namespace tinyoal;

#ifdef BSS_COMPILER_MSC
#if defined(BSS_DEBUG) && defined(BSS_CPU_x86_64)
#pragma comment(lib, "../lib/TinyOAL_d.lib")
#elif defined(BSS_CPU_x86_64)
#pragma comment(lib, "../lib/TinyOAL.lib")
#elif defined(BSS_DEBUG)
#pragma comment(lib, "../lib/TinyOAL32_d.lib")
#else
#pragma comment(lib, "../lib/TinyOAL32.lib")
#endif
#endif

TinyOALSystem::TinyOALSystem(unsigned char defnumbuf, const char* forceOAL, const char* forceOGG, const char* forceFLAC, const char* forceMP3) :
  cTinyOAL(defnumbuf, &Log, forceOAL, forceOGG, forceFLAC, forceMP3)
{
}
TinyOALSystem::~TinyOALSystem() {}
void TinyOALSystem::Process()
{
  Update(); // TODO: put this on a seperate thread.
}
int TinyOALSystem::Log(const char* file, unsigned int line, unsigned char level, const char* format, va_list args)
{
  return mgEngine::Instance()->GetLog().PrintLogV("tinyoal", file, line, level, format, args);
}
