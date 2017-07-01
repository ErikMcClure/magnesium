// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __CONTROL_MAP_H__MG__
#define __CONTROL_MAP_H__MG__

#include "mg_dec.h"
#include "bss-util/DynArray.h"
#include "bss-util/Hash.h"
#include <functional>

struct _FG_MSG;

namespace magnesium {
  // A helper class for mapping controls to actions designed to work with featherGUI
  class MG_DLLEXPORT mgControlMap
  {
    struct ControlAxis
    {
      std::function<void(float)> f;
      float value;
      float button; // Value used for button bound to the positive or negative axis.
      size_t id;
    };
    
    struct ControlButton
    {
      std::function<void(bool)> f;
      bool down;
      float threshold; // an axis must exceed this threshold to activate the button. If positive, it must be more positive, if negative, it must be more negative.
      size_t id;
    };

  public:
    mgControlMap();
    ~mgControlMap();
    size_t Message(const struct _FG_MSG* msg);
    void BindAxis(size_t id, uint8_t cancelkeycode = DEFAULT_CANCEL);
    void BindAxisButton(size_t id, bool positive, uint8_t cancelkeycode = DEFAULT_CANCEL);
    void BindButton(size_t id, uint8_t cancelkeycode = DEFAULT_CANCEL);
    float GetAxis(size_t id) const;
    bool GetButton(size_t id) const;
    bool AddAxis(size_t id, std::function<void(float)> f, float button);
    bool AddButton(size_t id, std::function<void(bool)> f, float threshold);
    void SetKey(size_t id, uint8_t key, bool positive = true);
    void SetAxis(size_t id, short axis);
    void SetButton(size_t id, short joybutton, bool positive = true);

    static const uint8_t DEFAULT_CANCEL = 0x1B; // 0x1B is featherGUI's keycode for Escape
    static const short MOUSEX_AXIS = -3;
    static const short MOUSEY_AXIS = -2;

    inline bool operator[](size_t id) const { return GetButton(id); }
    inline float operator()(size_t id) const { return GetAxis(id); }

  protected:
    void _bind(size_t id, char type, uint8_t key);
    void _processbutton(uint16_t id, bool down);
    void _processaxis(uint16_t id, float value);

    bss::DynArray<ControlAxis, uint16_t, bss::ARRAY_SAFE> _axismap;
    bss::DynArray<ControlButton, uint16_t, bss::ARRAY_SAFE> _buttonmap;
    bss::Hash<size_t, uint16_t> _idmap;
    bss::Hash<short, uint16_t> _joybuttonmap;
    bss::Hash<short, uint16_t> _joyaxismap;
    uint16_t _keys[256];
    size_t _curbindid;
    char _curbindtype;
    uint8_t _curbindcancel;
  };
}

#endif