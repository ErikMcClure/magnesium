// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgControlMap.h"
#include "feathergui/feathergui.h"

using namespace magnesium;
using namespace bss;

mgControlMap::mgControlMap() : _curbind((ControlID)~0), _cancel(DEFAULT_CANCEL)
{
}
mgControlMap::~mgControlMap()
{
}
size_t mgControlMap::Message(const FG_Msg& msg)
{
  if(_curbind != (ControlID)~0) // if we are trying to bind a key, divert into alternate codepath and return 0 to eat the message
  {
    switch(msg.type)
    {
    case FG_KEYUP:
    case FG_MOUSEUP:
    case FG_MOUSEMOVE:
    case FG_JOYBUTTONUP:
      return 0;
    case FG_KEYDOWN:
      _bindings.keys[msg.keycode] = _curbind;
      break;
    case FG_MOUSEDOWN:
      switch(msg.button)
      {
      case FG_MOUSELBUTTON: _bindings.keys[FG_KEY_LBUTTON] = _curbind; break;
      case FG_MOUSERBUTTON: _bindings.keys[FG_KEY_RBUTTON] = _curbind; break;
      case FG_MOUSEMBUTTON: _bindings.keys[FG_KEY_MBUTTON] = _curbind; break;
      case FG_MOUSEXBUTTON1: _bindings.keys[FG_KEY_XBUTTON1] = _curbind; break;
      case FG_MOUSEXBUTTON2: _bindings.keys[FG_KEY_XBUTTON2] = _curbind; break;
      }
      break;
    case FG_JOYBUTTONDOWN:
      _bindings.joybuttonmap.Insert(msg.joybutton, _curbind);
      break;
    case FG_JOYAXIS:
      _bindings.joyaxismap.Insert(msg.joyaxis, _curbind);
      break;
    }

    _curbind = (ControlID)~0;
    return 1;
  }

  switch(msg.type)
  {
  case FG_KEYUP:
  case FG_KEYDOWN:
    if(_bindings.keys[msg.keycode] != (ControlID)~0)
      _processbutton(_bindings.keys[msg.keycode], msg.type == FG_KEYDOWN);
    break;
  case FG_MOUSEUP:
  case FG_MOUSEDOWN:
  {
    uint8_t code = bss::bssLog2_p2(msg.button) + FG_KEY_LBUTTON;
    if(code >= 3) ++code; // the last three buttons are interrupted by the FG_CANCEL key
    if(_bindings.keys[code] != (ControlID)~0)
      _processbutton(_bindings.keys[code], msg.type == FG_MOUSEDOWN);
  }
    break;
  case FG_MOUSEMOVE:
  {
    ControlID xindex = _bindings.joyaxismap[MOUSEX_AXIS];
    ControlID yindex = _bindings.joyaxismap[MOUSEY_AXIS];
    if(xindex != (ControlID)~0)
      _processaxis(xindex, msg.x);
    if(yindex != (ControlID)~0)
      _processaxis(yindex, msg.y);
  }
    break;
  case FG_JOYBUTTONUP:
  case FG_JOYBUTTONDOWN:
  {
    ControlID index = _bindings.joybuttonmap[msg.joybutton];
    if(index != (ControlID)~0)
      _processbutton(index, msg.type == FG_JOYBUTTONDOWN);
  }
    break;
  case FG_JOYAXIS:
  {
    ControlID index = _bindings.joyaxismap[msg.joyaxis];
    float value = msg.joyvalue;
    if(_bindings.deadzone > 0.0f)
    {
      if(abs(value) < _bindings.deadzone)
        value = 0.0f;
      else if(value > 0.0f)
        value = (value - _bindings.deadzone) / (1.0 - _bindings.deadzone);
      else if(value < 0.0f)
        value = (value + _bindings.deadzone) / (1.0 - _bindings.deadzone);
    }
    if(index != (ControlID)~0)
      _processaxis(index, value);
  }
    break;
  }

  return 1;
}
void mgControlMap::Bind(ControlID id, bool positive)
{
  _curbind = id | (positive ? 0 : CONTROL_NEGATIVE);
}
const mgControlMap::Control* mgControlMap::GetControl(ControlID id) const
{
  id &= (~CONTROL_NEGATIVE);
  if(id >= _controlmap.Length())
    return nullptr;
  return (_controlmap[id].type == CONTROL_INVALID) ? nullptr : (_controlmap.begin() + id);
}

bool mgControlMap::AddAxis(ControlID id, float button)
{
  Control c = { CONTROL_AXIS, button, 0.0f, 0.0f };
  return _addcontrol(id, c);
}
bool mgControlMap::AddButton(ControlID id,  float threshold)
{
  Control c = { CONTROL_BUTTON, threshold, 0.0f, 0.0f };
  return _addcontrol(id, c);
}
void mgControlMap::BindKey(ControlID id, uint8_t key, bool positive)
{
  assert(!(id&CONTROL_NEGATIVE));
  _bindings.keys[key] = id | (positive ? 0 : CONTROL_NEGATIVE);
}
void mgControlMap::BindAxis(ControlID id, short axis, bool positive)
{
  assert(!(id&CONTROL_NEGATIVE));
  _bindings.joyaxismap.Insert(axis, id | (positive ? 0 : CONTROL_NEGATIVE));
}
void mgControlMap::BindButton(ControlID id, short joybutton, bool positive)
{
  assert(!(id&CONTROL_NEGATIVE));
  _bindings.joybuttonmap.Insert(joybutton, id | (positive ? 0 : CONTROL_NEGATIVE));
}

void mgControlMap::_processbutton(ControlID id, bool down)
{
  bool negative = id&CONTROL_NEGATIVE;
  id &= (~CONTROL_NEGATIVE);
  if(id >= _controlmap.Length())
    return;

  Control& c = _controlmap[id];
  if(c.type == CONTROL_INVALID)
    return;

  if(c.type&CONTROL_AXIS)
  {
    float value = negative ? -c.threshold : c.threshold;
    if(down)
    {
      if((c.value == 0) || (c.value < 0) != negative)
        c.prev = _assignaxis(id, c, value);
    }
    else
    {
      if((c.value != 0) && (c.value < 0) == negative)
        _assignaxis(id, c, c.prev);
      c.prev = 0;
    }
  }
  else
    _assignbutton(id, c, down);
}
void mgControlMap::_processaxis(ControlID id, float value)
{
  bool negative = id&CONTROL_NEGATIVE;
  id &= (~CONTROL_NEGATIVE);
  if(id >= _controlmap.Length())
    return;

  Control& c = _controlmap[id];
  if(c.type == CONTROL_INVALID)
    return;

  c.prev = 0;
  if(c.type&CONTROL_AXIS)
    _assignaxis(id, c, negative ? -value : value);
  else
    _assignbutton(id, c, abs(value) > c.threshold);
}
float mgControlMap::_assignaxis(ControlID id, Control& c, float value)
{
  assert(!(id&CONTROL_NEGATIVE));
  assert(c.type&CONTROL_AXIS);
  float old = c.value;
  c.value = value;
  if(!_func.IsEmpty() && value != old)
    _func(id, old);
  return old;
}
void mgControlMap::_assignbutton(ControlID id, Control& c, bool down)
{
  assert(!(id&CONTROL_NEGATIVE));
  assert(!(c.type&CONTROL_AXIS));
  CONTROL_TYPE old = c.type;
  c.type = down ? CONTROL_BUTTON_DOWN : CONTROL_BUTTON_UP;
  if(!_func.IsEmpty() && c.type != old)
    _func(id, 0.0f);
}
bool mgControlMap::_addcontrol(ControlID id, const Control& control)
{
  assert(!(id&CONTROL_NEGATIVE));
  if(GetControl(id) != nullptr)
    return false;
  if(_controlmap.Length() <= id)
  {
    size_t old = _controlmap.Length();
    _controlmap.SetLength(id + 1);
    memset(_controlmap.begin() + old, ~0, _controlmap.Length() - old);
  }
  _controlmap[id] = control;
  return true;
}

