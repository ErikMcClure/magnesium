// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __CONSOLE_H__MG__
#define __CONSOLE_H__MG__

#include "feathergui/fgText.h"
#include "bss-util/DynArray.h"
#include <sstream>

struct _FG_ELEMENT;

namespace magnesium {
  // A log target that records the entire history to a string and displays it on a GUI element
  class mgConsole : public std::stringbuf
  {
  public:
    mgConsole();
    ~mgConsole();
    void Scroll(ptrdiff_t diff);
    void ScrollReset();
    void Clear();
    void Load(fgElement* BSS_RESTRICT parent, fgElement* BSS_RESTRICT next, const char* name, fgFlag flags, const fgTransform* transform, fgMsgType units);
    void Destroy();
    inline fgElement* GetElement() { return _console; }
    inline size_t GetMaxLines() const { return _maxlines; }

    static size_t ConsoleMessage(fgText* self, const FG_Msg* msg);

  protected:
    virtual int sync() override;
    void _recalcLines();
    void _writeConsole();

    fgText _console;
    size_t _offset;
    size_t _maxlines;
    size_t _len;
    bss::DynArray<size_t> _lines;
  };
}

#endif