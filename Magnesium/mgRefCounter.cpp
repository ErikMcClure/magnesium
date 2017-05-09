// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgRefCounter.h"

using namespace magnesium;
using namespace bss;

int mgRefCounter::_refdefer = 0;
mgRefCounter* mgRefCounter::_reflast = nullptr;
Hash<mgRefCounter*, mgRefCounter*> mgRefCounter::_refbuffer;

int mgRefCounter::DropAll()
{
  if(--_refdefer <= 0)
  {
    mgRefCounter* cur = _reflast;
    while(cur != nullptr)
    {
      if(cur->_refcount <= 0)
        cur->DestroyThis();
      cur = _refbuffer[cur];
    }
    _reflast = nullptr;
    _refbuffer.Clear();
  }

  return _refdefer;
}