// Copyright ©2016 Black Sphere Studios
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

#define PSLOG(level) BSSLOG(*psEngine::Instance(),level)
#define PSLOGV(level,...) BSSLOGV(*psEngine::Instance(),level,__VA_ARGS__)

namespace planeshader {
  class psPass;
  class psDriver;
  class psRoot;

  struct PSINIT
  {
    inline PSINIT() : width(0), height(0), driver(RealDriver::DRIVERTYPE_DX11), mode(MODE_WINDOWED), vsync(false), sRGB(false),
      antialias(0), errout(0), mediapath("") {}

    int width;
    int height;
    RealDriver::DRIVERTYPE driver;
    enum MODE : char {
      MODE_WINDOWED = 0,
      MODE_FULLSCREEN,
      MODE_BORDERLESS_TOPMOST,
      MODE_BORDERLESS,
      MODE_COMPOSITE,
      MODE_COMPOSITE_CLICKTHROUGH,
      MODE_COMPOSITE_NOMOVE,
      MODE_COMPOSITE_OPAQUE_CLICK,
    } mode;
    bool vsync;
    bool sRGB;
    uint8_t antialias;
    std::ostream* errout;
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
  class PS_DLLEXPORT psEngine : public psGUIManager, public bss_util::cLog, public psDriverHold
  {
    static const uint8_t PSENGINE_QUIT = (1<<0);

  public:
    // Constructor
    psEngine(const PSINIT& init);
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
    // Get/Sets the quit value
    inline void Quit() { _flags+=PSENGINE_QUIT; }
    inline bool GetQuit() const { return _flags[PSENGINE_QUIT]; }
    inline const char* GetMediaPath() const { return _mediapath.c_str(); }
    inline PSINIT::MODE GetMode() const { return _mode; }
    void Resize(psVeciu dim, PSINIT::MODE mode);

    psPass& operator [](uint16_t index) { assert(index<_passes.Capacity()); return *_passes[index]; }
    const psPass& operator [](uint16_t index) const { assert(index<_passes.Capacity()); return *_passes[index]; }

    static psEngine* Instance(); // Cannot be inline'd for DLL reasons.

  protected:
    virtual void _onresize(uint32_t width, uint32_t height) override;

    bss_util::cArray<psPass*, uint16_t> _passes;
    bss_util::cBitField<uint8_t> _flags;
    uint16_t _curpass;
    psPass* _mainpass;
    cStr _mediapath;
    PSINIT::MODE _mode;
    uint64_t _frameprofiler;

    static psEngine* _instance;
  };
}

#endif