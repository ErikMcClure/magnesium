// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __CONTROL_MAP_H__MG__
#define __CONTROL_MAP_H__MG__

#include "mg_dec.h"
#include "bss-util/DynArray.h"
#include "bss-util/Hash.h"
#include "bss-util/Delegate.h"

struct _FG_MSG;

namespace magnesium {
  // A helper class for mapping controls to actions designed to work with featherGUI. Control IDs are stored in an array, not a hash, so avoid using huge ID numbers.
  class MG_DLLEXPORT mgControlMap
  {
    enum CONTROL_TYPE : uint8_t
    {
      CONTROL_BUTTON = 0,
      CONTROL_BUTTON_UP = 0,
      CONTROL_BUTTON_DOWN = 1,
      CONTROL_AXIS = 2,
      //CONTROL_AXIS_ABSOLUTE = 4,
      CONTROL_INVALID = (uint8_t)~0,
    };

    struct Control
    {
      CONTROL_TYPE type;
      float threshold; // For buttons controls, the absolute value an axis must exceed to activate it. For axis controls, the value a button corresponds to.
      float value; // For axis controls only, the current value.
      float prev; // For axis controls only, the previous value before a button was pressed.
    };

  public:
    typedef uint16_t ControlID;

    struct Bindings
    {
      Bindings() : deadzone(0.0f) { memset(keys, 0xFF, sizeof(ControlID) * 256); }

      float deadzone;
      bss::Hash<short, ControlID> joybuttonmap;
      bss::Hash<short, ControlID> joyaxismap;
      ControlID keys[256];

      template<typename Engine>
      void Serialize(bss::Serializer<Engine>& e, const char*)
      {
        e.EvaluateType<Bindings>(
          GenPair("deadzone", deadzone),
          GenPair("buttonmap", joybuttonmap),
          GenPair("axismap", joyaxismap),
          GenPair("keymap", keys)
          );
      }
    };

    mgControlMap();
    ~mgControlMap();
    size_t Message(const struct _FG_MSG& msg);
    void Bind(ControlID id, bool positive);
    BSS_FORCEINLINE float GetAxis(ControlID id) const { if(const Control* c = GetControl(id)) return c->value; return 0; }
    BSS_FORCEINLINE bool GetButton(ControlID id) const { if(const Control* c = GetControl(id)) return (c->type&CONTROL_BUTTON_DOWN); return false; }
    const Control* GetControl(ControlID id) const;
    bool AddAxis(ControlID id, float button);
    bool AddButton(ControlID id, float threshold);
    void BindKey(ControlID id, uint8_t key, bool positive = true);
    void BindAxis(ControlID id, short axis, bool positive = true);
    void BindButton(ControlID id, short joybutton, bool positive = true);
    inline void SetCancelKey(uint8_t cancel) { _cancel = cancel; }
    inline void SetFunction(bss::Delegate<void, ControlID, float> f) { _func = f; }
    inline const Bindings& GetBindings() const { return _bindings; }
    inline void SetBindings(const Bindings& bindings) { _bindings = bindings; }
    inline float GetDeadzone() const { return _bindings.deadzone; }
    inline void SetDeadzone(float deadzone) { assert(deadzone >= 0.0f); _bindings.deadzone = deadzone; }

    static const uint8_t DEFAULT_CANCEL = 0x1B; // 0x1B is featherGUI's keycode for Escape
    static const short MOUSEX_AXIS = -3;
    static const short MOUSEY_AXIS = -2;
    static const ControlID CONTROL_NEGATIVE = (1 << ((sizeof(ControlID) << 3) - 1));

    inline bool operator[](ControlID id) const { return GetButton(id); }
    inline float operator()(ControlID id) const { return GetAxis(id); }

  protected:
    void _processbutton(ControlID id, bool down);
    void _processaxis(ControlID id, float value);
    bool _addcontrol(ControlID id, const Control& control);
    float _assignaxis(ControlID id, Control& c, float value);
    void _assignbutton(ControlID id, Control& c, bool down);

    bss::DynArray<Control, size_t, bss::ARRAY_SIMPLE> _controlmap;
    bss::Delegate<void, ControlID, float> _func;
    Bindings _bindings;
    ControlID _curbind;
    uint8_t _cancel;
  };
}

#endif