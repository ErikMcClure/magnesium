// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgControlMap.h"
#include "feathergui/feathergui.h"

using namespace magnesium;
using namespace bss;

mgControlMap::mgControlMap() : _curbindid((uint16_t)-1), _curbindtype(0), _curbindcancel(DEFAULT_CANCEL)
{
  memset(_keys, 0xFF, sizeof(uint16_t) * 256);
}
mgControlMap::~mgControlMap()
{
}
size_t mgControlMap::Message(const FG_Msg* msg)
{
  if(_curbindid != (uint16_t)-1) // if we are trying to bind a key, divert into alternate codepath and return 0 to eat the message
  {
    switch(msg->type)
    {
    case FG_KEYUP:
    case FG_KEYDOWN:
      break;
    case FG_MOUSEUP:
    case FG_MOUSEDOWN:
    break;
    case FG_MOUSEMOVE:
      break;
    case FG_JOYBUTTONUP:
    case FG_JOYBUTTONDOWN:
      break;
    case FG_JOYAXIS:
      break;
    }

    _curbindid = (uint16_t)-1;
    return 0;
  }

  switch(msg->type)
  {
  case FG_KEYUP:
  case FG_KEYDOWN:
    if(_keys[msg->keycode] != (uint16_t)-1)
      _processbutton(_keys[msg->keycode], msg->type == FG_KEYDOWN);
    break;
  case FG_MOUSEUP:
  case FG_MOUSEDOWN:
  {
    uint8_t code = bss::bssLog2_p2(msg->button);
    if(code >= 3) ++code; // the last three buttons are interrupted by the FG_CANCEL key
    if(_keys[code] != (uint16_t)-1)
      _processbutton(_keys[code], msg->type == FG_MOUSEDOWN);
  }
    break;
  case FG_MOUSEMOVE:
  {
    uint16_t xindex = _joyaxismap[MOUSEX_AXIS];
    uint16_t yindex = _joyaxismap[MOUSEY_AXIS];
    if(xindex != (uint16_t)-1)
      _processaxis(xindex, msg->x);
    if(yindex != (uint16_t)-1)
      _processaxis(yindex, msg->y);
  }
    break;
  case FG_JOYBUTTONUP:
  case FG_JOYBUTTONDOWN:
  {
    uint16_t index = _joybuttonmap[msg->joybutton];
    if(index != (uint16_t)-1)
      _processbutton(index, msg->type == FG_MOUSEDOWN);
  }
    break;
  case FG_JOYAXIS:
    uint16_t index = _joyaxismap[msg->joyaxis];
    if(index != (uint16_t)-1)
      _processaxis(index, msg->joyvalue);
    break;
  }

  return 1;
}
void mgControlMap::BindAxis(size_t id, uint8_t cancelkeycode) { _bind(id, cancelkeycode, 1); } // TODO: binding an axis can use the same positive/negative flag to specify if it's absolute or relative.
void mgControlMap::BindAxisButton(size_t id, bool positive, uint8_t cancelkeycode) { _bind(id, cancelkeycode, 1 | (positive ? 2 : 0)); }
void mgControlMap::BindButton(size_t id, uint8_t cancelkeycode) { _bind(id, cancelkeycode, 0); }
float mgControlMap::GetAxis(size_t id) const
{
  uint16_t index = _idmap[id];
  return ((index != (uint16_t)-1) && ((index & 0x8000) != 0)) ? _axismap[index].value : 0.0f;
}
bool mgControlMap::GetButton(size_t id) const
{
  uint16_t index = _idmap[id];
  return ((index != (uint16_t)-1) && ((index & 0x8000) == 0)) ? _buttonmap[index].down : false;
}
bool mgControlMap::AddAxis(size_t id, std::function<void(float)> f, float button)
{
  if(_idmap.Exists(id))
    return false;
  uint16_t index = _axismap.Add(ControlAxis { f, 0.0f, button, id });
  assert(index < 0x8000);
  _idmap.Insert(id, index | 0x8000); // mark this as an axis
  return true;
}
bool mgControlMap::AddButton(size_t id, std::function<void(bool)> f, float threshold)
{
  if(_idmap.Exists(id))
    return false;
  uint16_t index = _buttonmap.Add(ControlButton { f, false, threshold, id });
  assert(index < 0x8000);
  _idmap.Insert(id, index);
  return true;
}
void mgControlMap::SetKey(size_t id, uint8_t key, bool positive)
{

}
void mgControlMap::SetAxis(size_t id, short axis)
{

}
void mgControlMap::SetButton(size_t id, short joybutton, bool positive)
{

}
void mgControlMap::_bind(size_t id, char type, uint8_t key)
{
  _curbindid = id;
  _curbindcancel = key;
  _curbindtype = type;
}

void mgControlMap::_processbutton(uint16_t id, bool down)
{

}
void mgControlMap::_processaxis(uint16_t id, float value)
{

}
