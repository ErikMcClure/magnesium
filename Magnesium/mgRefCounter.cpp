// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgRefCounter.h"

using namespace magnesium;
using namespace bss_util;

int mgRefCounter::_refdefer = 0;
mgRefCounter* mgRefCounter::_reflast = nullptr;
cHash<mgRefCounter*, mgRefCounter*> mgRefCounter::_refbuffer;

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
  }

  return _refdefer;
}