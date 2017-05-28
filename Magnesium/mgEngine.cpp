// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgEngine.h"

using namespace magnesium;

mgEngine* mgEngine::_instance = 0;
const char* mgEngine::LOGSOURCE = "magnesium";
const bssVersionInfo mgEngine::Version = { MG_VERSION_REVISION, MG_VERSION_MINOR, MG_VERSION_MAJOR };

mgEngine::mgEngine(std::ostream* log) : _timewarp(1.0), _log(!log ? "Magnesium.log" : 0, log) { _instance = this; }
mgEngine::mgEngine(const char* logfile) : _timewarp(1.0), _log(logfile) { _instance = this; }
mgEngine::~mgEngine() { _instance = 0; }

mgEngine* mgEngine::Instance() { return _instance; }
void mgEngine::UpdateDelta()
{
  if(_timewarp == 1.0)
    Update();
  else
    Update(_timewarp);
}

