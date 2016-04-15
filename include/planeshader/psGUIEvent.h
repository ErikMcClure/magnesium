// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __GUI_EVENT_H__PS__
#define __GUI_EVENT_H__PS__

#include "ps_dec.h"

namespace planeshader {
  //The keycodes enumerator
  enum KeyCodes : uint8_t
  {
    KEY_NULL = 0, //because its possible to have _lastkey Set to this
    KEY_LBUTTON      = 0x01,
    KEY_RBUTTON      = 0x02,
    KEY_CANCEL       = 0x03,
    KEY_MBUTTON      = 0x04,    /* NOT contiguous with L & RBUTTON */
    KEY_XBUTTON1     = 0x05,    /* NOT contiguous with L & RBUTTON */
    KEY_XBUTTON2     = 0x06,    /* NOT contiguous with L & RBUTTON */
    KEY_BACK         = 0x08,
    KEY_TAB          = 0x09,
    KEY_CLEAR        = 0x0C,
    KEY_RETURN       = 0x0D,
    KEY_SHIFT        = 0x10,
    KEY_CONTROL      = 0x11,
    KEY_MENU         = 0x12,
    KEY_PAUSE        = 0x13,
    KEY_CAPITAL      = 0x14,
    KEY_KANA         = 0x15,
    KEY_JUNJA        = 0x17,
    KEY_FINAL        = 0x18,
    KEY_KANJI        = 0x19,
    KEY_ESCAPE       = 0x1B,
    KEY_CONVERT      = 0x1C,
    KEY_NONCONVERT   = 0x1D,
    KEY_ACCEPT       = 0x1E,
    KEY_MODECHANGE   = 0x1F,
    KEY_SPACE        = 0x20,
    KEY_PRIOR        = 0x21,
    KEY_NEXT         = 0x22,
    KEY_END          = 0x23,
    KEY_HOME         = 0x24,
    KEY_LEFT         = 0x25,
    KEY_UP           = 0x26,
    KEY_RIGHT        = 0x27,
    KEY_DOWN         = 0x28,
    KEY_SELECT       = 0x29,
    KEY_PRINT        = 0x2A,
    //KEY_EXECUTE      = 0x2B,
    KEY_SNAPSHOT     = 0x2C,
    KEY_INSERT       = 0x2D,
    KEY_DELETE       = 0x2E,
    KEY_HELP         = 0x2F,
    KEY_0            = 0x30,
    KEY_1            = 0x31,
    KEY_2            = 0x32,
    KEY_3            = 0x33,
    KEY_4            = 0x34,
    KEY_5            = 0x35,
    KEY_6            = 0x36,
    KEY_7            = 0x37,
    KEY_8            = 0x38,
    KEY_9            = 0x39,
    KEY_A            = 0x41,
    KEY_B            = 0x42,
    KEY_C            = 0x43,
    KEY_D            = 0x44,
    KEY_E            = 0x45,
    KEY_F            = 0x46,
    KEY_G            = 0x47,
    KEY_H            = 0x48,
    KEY_I            = 0x49,
    KEY_J            = 0x4A,
    KEY_K            = 0x4B,
    KEY_L            = 0x4C,
    KEY_M            = 0x4D,
    KEY_N            = 0x4E,
    KEY_O            = 0x4F,
    KEY_P            = 0x50,
    KEY_Q            = 0x51,
    KEY_R            = 0x52,
    KEY_S            = 0x53,
    KEY_T            = 0x54,
    KEY_U            = 0x55,
    KEY_V            = 0x56,
    KEY_W            = 0x57,
    KEY_X            = 0x58,
    KEY_Y            = 0x59,
    KEY_Z            = 0x5A,
    KEY_LWIN         = 0x5B,
    KEY_RWIN         = 0x5C,
    KEY_APPS         = 0x5D,
    KEY_SLEEP        = 0x5F,
    KEY_NUMPAD0      = 0x60,
    KEY_NUMPAD1      = 0x61,
    KEY_NUMPAD2      = 0x62,
    KEY_NUMPAD3      = 0x63,
    KEY_NUMPAD4      = 0x64,
    KEY_NUMPAD5      = 0x65,
    KEY_NUMPAD6      = 0x66,
    KEY_NUMPAD7      = 0x67,
    KEY_NUMPAD8      = 0x68,
    KEY_NUMPAD9      = 0x69,
    KEY_MULTIPLY     = 0x6A,
    KEY_ADD          = 0x6B,
    KEY_SEPARATOR    = 0x6C,
    KEY_SUBTRACT     = 0x6D,
    KEY_DECIMAL      = 0x6E,
    KEY_DIVIDE       = 0x6F,
    KEY_F1           = 0x70,
    KEY_F2           = 0x71,
    KEY_F3           = 0x72,
    KEY_F4           = 0x73,
    KEY_F5           = 0x74,
    KEY_F6           = 0x75,
    KEY_F7           = 0x76,
    KEY_F8           = 0x77,
    KEY_F9           = 0x78,
    KEY_F10          = 0x79,
    KEY_F11          = 0x7A,
    KEY_F12          = 0x7B,
    KEY_F13          = 0x7C,
    KEY_F14          = 0x7D,
    KEY_F15          = 0x7E,
    KEY_F16          = 0x7F,
    KEY_F17          = 0x80,
    KEY_F18          = 0x81,
    KEY_F19          = 0x82,
    KEY_F20          = 0x83,
    KEY_F21          = 0x84,
    KEY_F22          = 0x85,
    KEY_F23          = 0x86,
    KEY_F24          = 0x87,
    KEY_NUMLOCK      = 0x90,
    KEY_SCROLL       = 0x91,
    KEY_OEM_NEC_EQUAL= 0x92,   // '=' key on numpad
    KEY_LSHIFT       = 0xA0,
    KEY_RSHIFT       = 0xA1,
    KEY_LCONTROL     = 0xA2,
    KEY_RCONTROL     = 0xA3,
    KEY_LMENU        = 0xA4,
    KEY_RMENU        = 0xA5,
    KEY_OEM_1        = 0xBA,   // ';:' for US
    KEY_OEM_PLUS     = 0xBB,   // '+' any country
    KEY_OEM_COMMA    = 0xBC,   // ',' any country
    KEY_OEM_MINUS    = 0xBD,   // '-' any country
    KEY_OEM_PERIOD   = 0xBE,   // '.' any country
    KEY_OEM_2        = 0xBF,   // '/?' for US
    KEY_OEM_3        = 0xC0,   // '`virtual ~' for US
    KEY_OEM_4        = 0xDB,  //  '[{' for US
    KEY_OEM_5        = 0xDC,  //  '\|' for US
    KEY_OEM_6        = 0xDD,  //  ']}' for US
    KEY_OEM_7        = 0xDE,  //  ''"' for US
    KEY_OEM_8        = 0xDF
  };

  //The joystick button enumerator
  enum JoyButtons : uint16_t
  {
    JOYBUTTON1=0,
    JOYBUTTON2=1,
    JOYBUTTON3=2,
    JOYBUTTON4=3,
    JOYBUTTON5=4,
    JOYBUTTON6=5,
    JOYBUTTON7=6,
    JOYBUTTON8=7,
    JOYBUTTON9=8,
    JOYBUTTON10=9,
    JOYBUTTON11=10,
    JOYBUTTON12=11,
    JOYBUTTON13=12,
    JOYBUTTON14=13,
    JOYBUTTON15=14,
    JOYBUTTON16=15,
    JOYBUTTON17=16,
    JOYBUTTON18=17,
    JOYBUTTON19=18,
    JOYBUTTON20=19,
    JOYBUTTON21=20,
    JOYBUTTON22=21,
    JOYBUTTON23=22,
    JOYBUTTON24=23,
    JOYBUTTON25=24,
    JOYBUTTON26=25,
    JOYBUTTON27=26,
    JOYBUTTON28=27,
    JOYBUTTON29=28,
    JOYBUTTON30=29,
    JOYBUTTON31=30,
    JOYBUTTON32=31,
    JOYSTICK_ID1=0x0000,
    JOYSTICK_ID2=0x0100,
    JOYSTICK_ID3=0x0200,
    JOYSTICK_ID4=0x0300,
    JOYSTICK_ID5=0x0400,
    JOYSTICK_ID6=0x0500,
    JOYSTICK_ID7=0x0600,
    JOYSTICK_ID8=0x0700,
    JOYSTICK_ID9=0x0800,
    JOYSTICK_ID10=0x0900,
    JOYSTICK_ID12=0x0A00,
    JOYSTICK_ID13=0x0B00,
    JOYSTICK_ID14=0x0C00,
    JOYSTICK_ID15=0x0D00,
    JOYSTICK_ID16=0x0E00,
    JOYSTICK_INVALID=0xFFFF,
    JOYAXIS_X=0,
    JOYAXIS_Y=1,
    JOYAXIS_Z=2,
    JOYAXIS_R=3,
    JOYAXIS_U=4,
    JOYAXIS_V=5
  };

  enum GUI_MOUSEBUTTONS : uint8_t
  {
    GUI_L_BUTTON=1,
    GUI_R_BUTTON=2,
    GUI_M_BUTTON=4,
    GUI_X_BUTTON1=8,
    GUI_X_BUTTON2=16,
    GUI_NUM_BUTTONS=5
  };

  enum GUI_EVENT : uint8_t
  {
    GUI_MOUSEDOWN,
    GUI_MOUSEDBLCLICK,
    GUI_MOUSEUP,
    GUI_MOUSEMOVE,
    GUI_MOUSESCROLL,
    GUI_MOUSELEAVE, //mouse has left the window
    GUI_KEYUP,
    GUI_KEYDOWN,
    GUI_KEYCHAR, //Passed in conjunction with keydown/up to differentiate a typed character from other keys.
    GUI_JOYBUTTONDOWN,
    GUI_JOYBUTTONUP,
    GUI_JOYAXIS,
    GUI_CUSTOM
  };

  struct BSS_COMPILER_DLLEXPORT psGUIEvent {
    union {
      struct {
        int x; int y; // Mouse events
        union {
          struct { uint8_t button; uint8_t allbtn; };
          short scrolldelta;
        };
      };
      struct {  // Keys
        int keychar; //Only used by KEYCHAR, represents a utf32 character
        uint8_t keycode; //only used by KEYDOWN/KEYUP, represents an actual keycode, not a character
        char sigkeys; // 1: shift, 2: ctrl, 4: alt, 8: held
      };
      struct { float joyvalue; short joyaxis; }; // JOYAXIS
      struct { char joydown; short joybutton; }; // JOYBUTTON
    };
    uint8_t type;
    uint64_t time;

    bool IsPressed() const { return (button&allbtn)!=0; }
    bool IsShiftDown() const { return (1&sigkeys)!=0; }
    bool IsCtrlDown() const { return (2&sigkeys)!=0; }
    bool IsAltDown() const { return (4&sigkeys)!=0; }
    bool IsHeld() const { return (8&sigkeys)!=0; }
  };
}

#endif