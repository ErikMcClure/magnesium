// Copyright ©2018 Black Sphere Studios

#include "mgConsole.h"
#include "feathergui/fgElement.h"

using namespace magnesium;

mgConsole::mgConsole() : _len(0), _offset(0), _maxlines(0)
{
  bss::bssFill(_console);
}
mgConsole::~mgConsole()
{
  Destroy();
}
void mgConsole::Load(fgElement* BSS_RESTRICT parent, fgElement* BSS_RESTRICT next, const char* name, fgFlag flags, const fgTransform* transform, fgMsgType units)
{
  fgText_Init(&_console, parent, next, name, flags, transform, units);
  _console->userdata = this;
  _console->message = (fgMessage)&mgConsole::ConsoleMessage;
  _recalcLines();
}

void mgConsole::Destroy()
{
  if(_console->message != nullptr)
    fgText_Destroy(&_console);
  bss::bssFill(_console);
}
void mgConsole::Scroll(ptrdiff_t diff)
{
  if(diff < 0 && size_t(-diff) > _offset)
    return ScrollReset();
  _offset += diff;
  _writeConsole();
}
void mgConsole::ScrollReset()
{
  _offset = 0;
  _writeConsole();
}
size_t mgConsole::ConsoleMessage(fgText* self, const FG_Msg* msg)
{
  mgConsole* p = (mgConsole*)(*self)->userdata;
  size_t r = fgText_Message(self, msg);

  switch(msg->type)
  {
  case FG_MOVE:
    if(!(msg->u2 & FGMOVE_PROPAGATE))
      p->_recalcLines();
    break;
  case FG_SETFONT:
  case FG_SETLINEHEIGHT:
    p->_recalcLines();
    break;
  }

  return r;
}
void mgConsole::Clear()
{
  str(std::string());
  _len = 0;
  _offset = 0;
}

int mgConsole::sync()
{
  size_t old = _len;
  _len = (size_t)(pptr() - pbase());
  size_t lines = _lines.Length();

  for(size_t i = old; i < _len; ++i)
  {
    const char* s = pbase() + i -1;
    assert(pbase()[i] != 0);
    if(pbase()[i] == '\n')
      _lines.Add(i);
  }

  if(_offset > 0)
    _offset += (_lines.Length() - lines);
  _writeConsole();
  return 0;
}

void mgConsole::_recalcLines()
{
  AbsRect area;
  ResolveRect(_console, &area);
  float lineheight = _console->GetLineHeight();
  if(lineheight != 0.0f)
    _maxlines = bss::fFastTruncate((area.bottom - area.top) / lineheight);
  _writeConsole();
}
void mgConsole::_writeConsole()
{
  if(!_lines.Length())
    return;
  assert(_offset < _lines.Length());
  if(_console->message != nullptr)
  {
    size_t endoffset = _offset + _maxlines;
    char* end = _offset > 0 ? (pbase() + _lines[_lines.Length() - _offset - 1]) : pptr();
    char* begin = endoffset < _lines.Length() ? (pbase() + _lines[_lines.Length() - endoffset - 1] + 1) : pbase();
    _console->SetText(end == begin ? "" : begin, end - begin);
  }
}