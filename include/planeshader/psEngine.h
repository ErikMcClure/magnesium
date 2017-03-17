// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __ENGINE_H__PS__
#define __ENGINE_H__PS__

#include "bss-util/cArray.h"
#include "bss-util/cBitField.h"
#include "bss-util/bss_log.h"
#include "bss-util/cStr.h"
#include "bss-util/cSerializer.h"
#include "psGUIManager.h"
#include "psDriver.h"

#define PSLOG(level,...) psEngine::Instance()->Log(__FILE__,__LINE__,(level),__VA_ARGS__)
#define PSLOGF(level,format,...) psEngine::Instance()->LogFormat(__FILE__,__LINE__,(level),format,__VA_ARGS__)
#define PSLOGP(level,format,...) psEngine::Instance()->PrintLog(__FILE__,__LINE__,(level),format,__VA_ARGS__)

namespace planeshader {
  class psPass;
  class psDriver;
  class psRoot;

  struct PSINIT
  {
    inline PSINIT() : width(0), height(0), driver(RealDriver::DRIVERTYPE_DX11), mode(psMonitor::MODE_WINDOWED), vsync(false), sRGB(false),
      antialias(0), mediapath("") {}

    int width;
    int height;
    RealDriver::DRIVERTYPE driver;
    psMonitor::MODE mode;
    bool vsync;
    bool sRGB;
    uint8_t antialias;
    const char* mediapath;

    template<typename Engine>
    void Serialize(bss_util::cSerializer<Engine>& e)
    {
      e.EvaluateType<PSINIT>(
        GenPair("width", width),
        GenPair("height", height),
        GenPair("driver", (uint8_t&)driver),
        GenPair("mode", (char&)mode),
        GenPair("vsync", vsync),
        GenPair("sRGB", sRGB),
        GenPair("antialias", antialias)
        );
    }
  };

  // Core engine object
  class PS_DLLEXPORT psEngine : public psGUIManager, public psDriverHold
  {
  public:
    // Constructor
    psEngine(const PSINIT& init, std::ostream* log = 0);
    ~psEngine();
    // Begins a frame. Returns false if rendering should stop.
    bool Begin();
    bool Begin(uint32_t clearcolor);
    // Ends a frame, returning the number of nanoseconds between Begin() and End(), before the vsync happened.
    uint64_t End();
    // Renders the next pass, returns false if there are no more passes to render.
    bool NextPass();
    // Begins and ends a frame, returning false if rendering should stop.
    inline bool Render() {
      if(!Begin()) return false;
      End();
      FlushMessages(); // For best results, calculate the frame delta right before flushing the messages
      return true;
    }
    // Insert a pass 
    bool InsertPass(psPass& pass, uint16_t index=-1);
    // Remove pass 
    bool RemovePass(uint16_t index);
    // Gets a pass. The 0th pass always exists.
    inline psPass* GetPass(uint16_t index=0) const { return index<_passes.Capacity()?_passes[index]:0; }
    inline uint16_t NumPass() const { return _passes.Capacity(); }
    inline const char* GetMediaPath() const { return _mediapath.c_str(); }
    inline psMonitor::MODE GetMode() const { return _mode; }
    inline bss_util::cLog& GetLog() { return _log; }

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

    psPass& operator [](uint16_t index) { assert(index<_passes.Capacity()); return *_passes[index]; }
    const psPass& operator [](uint16_t index) const { assert(index<_passes.Capacity()); return *_passes[index]; }

    static psEngine* Instance(); // Cannot be inline'd for DLL reasons.
    static const char* LOGSOURCE;

  protected:
    virtual void _onresize(psVeciu dim, bool fullscreen) override;

    bss_util::cArray<psPass*, uint16_t> _passes;
    uint16_t _curpass;
    psPass* _mainpass;
    cStr _mediapath;
    psMonitor::MODE _mode;
    uint64_t _frameprofiler;
    bss_util::cLog _log;

    static psEngine* _instance;
  };
}

#endif