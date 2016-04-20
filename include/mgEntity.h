// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __ENTITY_H__MG__
#define __ENTITY_H__MG__

#include "mgRefCounter.h"
#include "bss-util/cMap.h"
#include "bss-util/LLBase.h"

namespace magnesium {
  typedef unsigned short ComponentID;
  struct MG_DLLEXPORT ComponentBitfield
  {
    uint64_t bits;
    inline ComponentBitfield& operator +=(ComponentID id) { bits |= (1ULL << id); return *this; }
    inline ComponentBitfield& operator -=(ComponentID id) { bits &= (~(11ULL << id)); return *this; }
    inline bool operator[](ComponentID id) { return (bits & (11ULL << id)) != 0; }
    inline bool operator==(ComponentBitfield& r) { return (bits&r.bits) == r.bits; }
  };

  static const ComponentBitfield COMPONENTBITFIELD_EMPTY = { 0 };

#ifdef BSS_DEBUG
  template<class T> // This is a debug tracking class that ensures you never add or remove a component
  struct mgComponentRef // while you have an active reference to it, because the pointer gets invalidated
  {
    mgComponentRef(const mgComponentRef& r) : ref(r.ref) { ++Counter(); }
    mgComponentRef(T* r) : ref(r) { ++Counter(); }
    ~mgComponentRef() { --Counter(); }
    T* operator ->() { return ref; }
    const T* operator ->() const { return ref; }
    T& operator *() { return ref; }
    const T& operator *() const { return ref; }
    operator T*() { return ref; }

    static int& Counter() { static int count = 0; return count; }
    T* ref;
  };
#define COMPONENT_REF(T) mgComponentRef<T>
#else
#define COMPONENT_REF(T) T*
#endif

  struct MG_DLLEXPORT mgEntity : public mgRefCounter, public bss_util::LLBase<mgEntity>
  {
    mgEntity();
    mgEntity(mgEntity&& mov);
    virtual ~mgEntity();
    void ComponentListInsert(ComponentID id, size_t);
    void ComponentListRemove(ComponentID id);
    size_t& ComponentListGet(ComponentID id);
    template<class T> // Gets a component of type T if it belongs to this entity, otherwise returns NULL
    COMPONENT_REF(T) Get() { ComponentID index = _componentlist.Get(T::ID()); return index == (ComponentID)~0 ? nullptr : T::Store().Get(_componentlist[index]); }
    template<class T> // Adds a component of type T to this entity
    COMPONENT_REF(T) Add() { T::Store().Add(this); return Get<T>(); }
    template<class T> // Removes a component of type T from this entity
    bool Remove() { ComponentID index = _componentlist.Get(T::ID()); if(index != (ComponentID)~0) return T::Store().Remove(_componentlist[index]); }

    size_t id;
    ComponentBitfield components; // bitfield of what components this entity has.

    inline static mgEntity* GetEntityList() { return _entitylist; }

  protected: // You can't interact with componentlist directly because it violates DLL bounderies
    bss_util::cMap<ComponentID, size_t, bss_util::CompT<ComponentID>, ComponentID> _componentlist;

    static mgEntity* _entitylist;
  };
}

#endif