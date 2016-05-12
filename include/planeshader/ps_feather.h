// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __FEATHER_H__PS__
#define __FEATHER_H__PS__

#include "feathergui\fgRoot.h"
#include "feathergui\fgMonitor.h"
#include "psRenderable.h"

struct HWND__;
struct HINSTANCE__;
struct HICON__;
struct tagPOINTS;

#if defined(_WIN64)
typedef int64_t longptr_t;
#else
typedef __w64 long longptr_t;
#endif

namespace planeshader {
  class psGUIManager;

  class PS_DLLEXPORT psRoot : public fgRoot, public psDriverHold, public psRenderable
  {
  public:
    psRoot();
    ~psRoot();
    
    static psFlag GetDrawFlags(fgFlag flags);

  protected:
    void BSS_FASTCALL _render();
  };

  class PS_DLLEXPORT psMonitor : public fgMonitor
  {
    static const uint8_t PSMONITOR_OVERRIDEHITTEST = (1 << 0);
    static const uint8_t PSMONITOR_HOOKNC = (1 << 1);
    static const uint8_t PSMONITOR_LOCKCURSOR = (1 << 2);
    static const uint8_t PSMONITOR_ISINSIDE = (1 << 3);
    static const uint8_t PSMONITOR_AUTOMINIMIZE = (1 << 4);
    static const uint8_t PSMONITOR_QUIT = (1 << 5);

  public:
    enum MODE : char {
      MODE_WINDOWED = 0,
      MODE_FULLSCREEN,
      MODE_BORDERLESS_TOPMOST,
      MODE_BORDERLESS,
      MODE_COMPOSITE,
      MODE_COMPOSITE_CLICKTHROUGH,
      MODE_COMPOSITE_NOMOVE,
      MODE_COMPOSITE_OPAQUE_CLICK,
    };

    psMonitor();
    psMonitor(psGUIManager* manager, psVeciu& dim, MODE mode, HWND__* window = 0);
    ~psMonitor();
    // Gets the window handle
    HWND__* GetWindow() const { return _window; }
    // Locks the cursor
    void LockCursor(bool lock);
    // Set window title
    void SetWindowTitle(const char* caption);
    inline MODE GetMode() const { return _mode; }
    void Resize(psVeciu dim, MODE mode);
    inline void SetBackBuffer(psTex* backbuffer) { _backbuffer = backbuffer; }
    inline psTex* const* GetBackBuffer() { return !_backbuffer ? 0 : &_backbuffer; }

  protected:
    psVeciu _resizewindow(psVeciu dim, char mode);
    HWND__* WndCreate(HINSTANCE__* instance, psVeciu dim, char mode, const wchar_t* icon, HICON__* iconrc);

    static size_t FG_FASTCALL Message(fgMonitor* self, const FG_Msg* m);
    static longptr_t __stdcall WndProc(HWND__* hWnd, uint32_t message, size_t wParam, longptr_t lParam);
    static tagPOINTS* __stdcall _STCpoints(HWND__* hWnd, tagPOINTS* target);
    static void _lockcursor(HWND__* hWnd, bool lock);

    HWND__* _window;
    psGUIManager* _manager;
    psTex* _backbuffer;
    MODE _mode;
    bss_util::cBitField<uint8_t> _guiflags;
  };
}

#endif