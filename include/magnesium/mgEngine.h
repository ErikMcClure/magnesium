// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __ENGINE_H__MG__
#define __ENGINE_H__MG__

#include "mgSystemManager.h"
#include "bss-util/HighPrecisionTimer.h"
#include "bss-util/Logger.h"
#include "bss-util/BlockAllocMT.h"
#include "bss-util/CacheAlloc.h"

#define MGLOG(level,...) mgEngine::Instance()->Log(__FILE__,__LINE__,(level),__VA_ARGS__)
#define MGLOGF(level,format,...) mgEngine::Instance()->LogFormat(__FILE__,__LINE__,(level),format,__VA_ARGS__)
#define MGLOGP(level,format,...) mgEngine::Instance()->PrintLog(__FILE__,__LINE__,(level),format,__VA_ARGS__)

namespace magnesium {
  class MG_DLLEXPORT mgEngine : public mgSystemManager, public bss::HighPrecisionTimer
  {
  public:
    explicit mgEngine(std::ostream* log = 0);
    explicit mgEngine(const char* logfile);
    ~mgEngine();
    void UpdateDelta();
    inline double GetTimewarp() const { return _timewarp; }
    inline void SetTimewarp(double timewarp) { _timewarp = timewarp; }
    inline mgEntity& SceneGraph() { return _root; }
    inline bss::Logger& GetLog() { return _log; }
    BSS_FORCEINLINE void Process() { mgSystemManager::Process(); }
    void Process(uint64_t delta);

    inline int PrintLog(const char* file, uint32_t line, uint8_t level, const char* format, ...)
    {
      va_list vl;
      va_start(vl, format);
      int r = _log.PrintLogV(LOGSOURCE, file, line, level, format, vl);
      va_end(vl);
      return r;
    }
    inline int PrintLogV(const char* file, uint32_t line, uint8_t level, const char* format, va_list args) { return _log.PrintLog(LOGSOURCE, file, line, level, format, args); }
    template<typename... Args>
    BSS_FORCEINLINE void Log(const char* file, uint32_t line, uint8_t level, Args... args) { _log.Log<Args...>(LOGSOURCE, file, line, level, args...); }
    template<typename... Args>
    BSS_FORCEINLINE void LogFormat(const char* file, uint32_t line, uint8_t level, const char* format, Args... args) { _log.LogFormat<Args...>(LOGSOURCE, file, line, level, format, args...); }

    static mgEngine* Instance();
    static const char* LOGSOURCE;
    static const bssVersionInfo Version;

  protected:
    static mgEngine* _instance;
    double _timewarp;
    uint64_t _override;
    bss::Logger _log;
    mgEntity _root;
  };
}

#endif