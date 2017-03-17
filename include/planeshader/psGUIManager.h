// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __GUI_MANAGER_H__PS__
#define __GUI_MANAGER_H__PS__

#include "bss-util/cBitField.h"
#include "bss-util/delegate.h"
#include "psVec.h"
#include "ps_feather.h"

namespace planeshader {
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
  public:
    psGUIManager();
    ~psGUIManager();
    // Returns true if the key designated by the code is pushed down 
    inline bool GetKey(FG_KEYS code) const { return (_allkeys[code] & 0x80)!=0; }
    // Returns true if joystick button designated by the code (the joystick ID plus the button ID, e.g. JOYSTICK_ID1|JOYBUTTON1) is pushed down 
    inline bool GetButton(uint16_t button) const { return (_allbuttons[button>>8]&(1<<(button&0xFF)))!=0; }
    // Returns a normalized relative axis value between -1.0 and 1.0 for the given axis code (the joystick ID plus the axis ID, e.g. JOYSTICK_ID1|JOYAXIS_X) 
    inline float GetAxis(uint16_t axis) const { return _translatejoyaxis(axis); }
    // Gets the mouse coordinates 
    inline psVec GetMouse() const { return psVec(_root.mouse.x, _root.mouse.y); }
    // Gets the ID of the first joystick that is plugged in, or JOYSTICKID_INVALID (-1) otherwise 
    inline FG_JOYBUTTONS GetValidJoystickID() const { return FG_JOYBUTTONS(_firstjoystick); }
    // Captures all joysticks that are plugged in and updates joystick validity bits. Returns number of joysticks that are connected 
    char CaptureAllJoy(HWND__*);
    // Tells you if a joystick is plugged in. 
    inline bool HasJoystick(uint8_t joy) const { return (_alljoysticks&(1<<joy))!=0; }
    // Gets the nth monitor we've initialized, where a monitor is either fullscreen or a window on the desktop.
    inline psMonitor* GetMonitor(uint8_t index = 0) { return index < _monitors.Length() ? &_monitors[index] : 0; }
    psMonitor* AddMonitor(psVeciu& dim, psMonitor::MODE mode, HWND__* window);
    // Shows/hides the hardware cursor
    void ShowCursor(bool show);
    // Flush the message queue
    void FlushMessages();
    // Get GUI root
    psRoot& GetGUI() { return _root; }
    // Gets the timestamp of the last message that was processed
    inline unsigned long GetLastMsgTime() const { return _lastmsgtime; }
    // Get/Sets the quit value
    inline void Quit() { _quit = true; }
    inline bool GetQuit() const { return _quit; }
    static psVeciu GetMonitorDPI(int);

    friend class psMonitor;
    static const uint16_t NUMKEYS=256; //256 possible keyboard IDs
    static const uint8_t NUMJOY=16; //Windows supports a max of 15 joysticks but the drivers claim they can support 16.
    static const uint8_t NUMAXIS=JOY_DEVCAPS::NUMAXIS; //Max number of axis accounted for
    static const uint32_t BASE_DPI = 96;

  protected:
    // Creates the window and actually sets everything up (otherwise we get pointer problems)
    size_t SetKey(uint8_t keycode, bool down, bool held, unsigned long time);
    void SetChar(int key, unsigned long time);
    void SetMouse(tagPOINTS* points, unsigned short type, unsigned char button, size_t wparam, unsigned long time);
    // Updates values for all plugged in joysticks 
    void _joyupdateall();
    // Translates joystick axis value to a [-1.0,1.0] range 
    float _translatejoyaxis(uint16_t axis) const;
    void _exactmousecalc();
    void _updaterootarea();
    virtual void _onresize(psVeciu dim, bool fullscreen) = 0;
    size_t _injectdefault(const FG_Msg&);

    static size_t Message(fgRoot* self, const FG_Msg* m);

    uint8_t _allkeys[NUMKEYS]; //holds keyboard layout state
    uint32_t _allbuttons[NUMJOY];
    uint16_t _firstjoystick; //Joystick ID of whatever the first plugged in joystick is
    uint16_t _alljoysticks; //Each bit, if set to 1, indicates the joystick corresponding to the bit position is connected
    unsigned long _alljoyaxis[NUMJOY][NUMAXIS]; //1 - x, 2 - y, 3 - z, 4 - r, 5 - u, 6 - v
    JOY_DEVCAPS _joydevs[NUMJOY];
    uint8_t _maxjoy; //Number of joysticks supported by the driver
    bool _quit;
    bss_util::cDynArray<psMonitor, uint8_t, bss_util::CARRAY_CONSTRUCT> _monitors;
    psRoot _root;
    unsigned long _lastmsgtime; // Gets the timestamp of the last message
  };
}

#endif