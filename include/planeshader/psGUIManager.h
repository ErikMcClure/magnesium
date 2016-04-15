// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __GUI_MANAGER_H__PS__
#define __GUI_MANAGER_H__PS__

#include "bss-util/cBitField.h"
#include "bss-util/delegate.h"
#include "psGUIEvent.h"
#include "psVec.h"

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
  struct BSS_COMPILER_DLLEXPORT cMouseData
  {
    inline cMouseData() : scrolldelta(0) {}
    psVeci relcoord;
    bss_util::cBitField<uint8_t> button; //use the mousebuttons enum for the corresponding array index here to see if that button is pressed (true=pressed, false=not pressed)
    short scrolldelta; //The last change in the scroll wheel
  };

  struct BSS_COMPILER_DLLEXPORT JOY_DEVCAPS
  {
    static const uint8_t NUMAXIS=6; //Max number of axis accounted for

    long offset[NUMAXIS];
    float range[NUMAXIS];
    uint8_t numaxes;
    uint8_t numbuttons;
  };

  // Manages the GUI window the graphics engine lives in and tracking all of the related input
  class PS_DLLEXPORT psGUIManager
  {
  protected:
    static const uint8_t PSGUIMANAGER_OVERRIDEHITTEST = (1<<0);
    static const uint8_t PSGUIMANAGER_HOOKNC = (1<<1);
    static const uint8_t PSGUIMANAGER_LOCKCURSOR = (1<<2);
    static const uint8_t PSGUIMANAGER_ISINSIDE = (1<<3);
    static const uint8_t PSGUIMANAGER_AUTOMINIMIZE = (1 << 4);

  public:
    psGUIManager();
    ~psGUIManager();
    // Returns true if the key designated by the code is pushed down 
    inline bool GetKey(KeyCodes code) const { return (_allkeys[code] & 0x80)!=0; }
    // Returns true if joystick button designated by the code (the joystick ID plus the button ID, e.g. JOYSTICK_ID1|JOYBUTTON1) is pushed down 
    inline bool GetButton(uint16_t button) const { return (_allbuttons[button>>8]&(1<<(button&0xFF)))!=0; }
    // Returns a normalized relative axis value between -1.0 and 1.0 for the given axis code (the joystick ID plus the axis ID, e.g. JOYSTICK_ID1|JOYAXIS_X) 
    inline float GetAxis(uint16_t axis) const { return _translatejoyaxis(axis); }
    // Gets general mouse information 
    const cMouseData& GetMouseData() const;
    // Gets the cGUIReceiver class used for catching input 
    inline const bss_util::delegate<bool, const psGUIEvent&>& GetInputReceiver() const { return _receiver; }
    // Sets the cInputReceiver class used for catching input 
    inline void SetInputReceiver(const bss_util::delegate<bool, const psGUIEvent&>& receiver) { _receiver=receiver; }
    // Gets the mouse coordinates 
    inline const psVeci& GetMouse() const { return _mousedata.relcoord; }
    // Gets the ID of the first joystick that is plugged in, or JOYSTICKID_INVALID (-1) otherwise 
    inline JoyButtons GetValidJoystickID() const { return JoyButtons(_firstjoystick); }
    // Captures all joysticks that are plugged in and updates joystick validity bits. Returns number of joysticks that are connected 
    char CaptureAllJoy(HWND__*);
    // Tells you if a joystick is plugged in. 
    inline bool HasJoystick(uint8_t joy) const { return (_alljoysticks&(1<<joy))!=0; }
    // Gets the window handle
    HWND__* GetWindow() const { return _window; }
    // Locks the cursor
    void LockCursor(bool lock);
    // Shows/hides the hardware cursor
    void ShowCursor(bool show);
    // Flush the message queue
    void FlushMessages();
    // Set window title
    void SetWindowTitle(const char* caption);

    static psVeciu GetMonitorDPI(int num = -1);

    static const uint16_t NUMKEYS=256; //256 possible keyboard IDs
    static const uint16_t NUMMOUSE=GUI_NUM_BUTTONS; //5 mouse buttons
    static const uint8_t NUMJOY=16; //Windows supports a max of 15 joysticks but the drivers claim they can support 16.
    static const uint8_t NUMAXIS=JOY_DEVCAPS::NUMAXIS; //Max number of axis accounted for

  protected:
    // Creates the window and actually sets everything up (otherwise we get pointer problems)
    psVeciu _create(psVeciu dim, char mode, HWND__* window);
    void SetKey(uint8_t keycode, bool down, bool held, unsigned long time);
    void SetChar(int key, unsigned long time);
    void SetMouse(tagPOINTS* points, uint8_t click, size_t wparam, unsigned long time);
    // Updates values for all plugged in joysticks 
    void _joyupdateall();
    // Translates joystick axis value to a [-1.0,1.0] range 
    float _translatejoyaxis(uint16_t axis) const;
    void _exactmousecalc();
    psVeciu _resizewindow(psVeciu dim, char mode);
    virtual void _onresize(uint32_t width, uint32_t height)=0;

    static HWND__* WndCreate(HINSTANCE__* instance, psVeciu dim, char mode, const wchar_t* icon, HICON__* iconrc);
    static longptr_t __stdcall WndProc(HWND__* hWnd, uint32_t message, size_t wParam, longptr_t lParam);
    static void _lockcursor(HWND__* hWnd, bool lock);
    static void _dolockcursor(HWND__* hWnd);
    static tagPOINTS* __stdcall _STCpoints(HWND__* hWnd, tagPOINTS* target);
    
    cMouseData _mousedata;
    uint8_t _allkeys[NUMKEYS]; //holds keyboard layout state
    uint32_t _allbuttons[NUMJOY];
    bss_util::delegate<bool, const psGUIEvent&> _receiver;
    //cControlManager _controls; //manages controls;
    uint16_t _firstjoystick; //Joystick ID of whatever the first plugged in joystick is
    uint16_t _alljoysticks; //Each bit, if set to 1, indicates the joystick corresponding to the bit position is connected
    unsigned long _alljoyaxis[NUMJOY][NUMAXIS]; //1 - x, 2 - y, 3 - z, 4 - r, 5 - u, 6 - v
    JOY_DEVCAPS _joydevs[NUMJOY];
    uint8_t _maxjoy; //Number of joysticks supported by the driver
    HWND__* _window;
    bss_util::cBitField<uint8_t> _guiflags;
  };
}

#endif