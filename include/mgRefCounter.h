// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __REF_COUNTER_H__MG__
#define __REF_COUNTER_H__MG__

#include "mg_dec.h"
#include "bss-util/cHash.h"

namespace magnesium {
  class MG_DLLEXPORT mgRefCounter
  {
  public:
    // Increments and returns the reference counter
    BSS_FORCEINLINE int Grab() { return ++_refcount; } const
    // Decrements the reference counter, destroys the object if it is equal to or less then 0, or buffers it if _defer is greater than 0
    BSS_FORCEINLINE int Drop()
    {
      int r = --_refcount;
      if(r <= 0)
      {
        if(_refdefer <= 0)
          DestroyThis();
        else if(!_refbuffer.Exists(this))
        {
          _refbuffer.Insert(this, _reflast);
          _reflast = this; // this lets us build a linked list inside the hash for efficient iteration
        }
      }
      return r;
    }

    static inline int GrabAll() { return ++_refdefer; }
    static int DropAll();

  protected:
    // Constructor - The count can be zero because DropAll() only touches reference counters that had Drop() called on them.
    inline mgRefCounter() { _refcount = 0; }
    inline mgRefCounter(const mgRefCounter& copy) { _refcount = 0; }
    // Destructor - Does nothing, but because it is virtual, ensures that all superclasses get destroyed as well
    virtual ~mgRefCounter() { }
    // Destroys this object - made a seperate virtual function so it is overridable to ensure it is deleted in the proper DLL
    virtual void DestroyThis() { delete this; }

    int _refcount;
    
    static int _refdefer;
    static mgRefCounter* _reflast;
    static bss_util::cHash<mgRefCounter*, mgRefCounter*> _refbuffer;
  };
}

#endif
