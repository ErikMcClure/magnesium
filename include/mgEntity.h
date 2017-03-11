// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __ENTITY_H__MG__
#define __ENTITY_H__MG__

#include "mgRefCounter.h"
#include "bss-util/cMap.h"
#include "bss-util/cCompactArray.h"

namespace magnesium {
  typedef unsigned short ComponentID;

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

  struct MG_DLLEXPORT mgEntity : public mgRefCounter
  {
    explicit mgEntity(mgEntity* parent = 0, int order = 0);
    mgEntity(mgEntity&& mov);
    virtual ~mgEntity();
    void ComponentListInsert(ComponentID id, ComponentID graphid, size_t);
    void ComponentListRemove(ComponentID id, ComponentID graphid);
    size_t& ComponentListGet(ComponentID id);
    template<class T> // Gets a component of type T if it belongs to this entity, otherwise returns NULL
    inline COMPONENT_REF(T) Get() { ComponentID index = _componentlist.Get(T::ID()); return index == (ComponentID)~0 ? nullptr : T::Store().Get(_componentlist[index]); }
    template<class T> // Adds a component of type T to this entity
    inline COMPONENT_REF(T) Add() { T::Store().Add(this); return Get<T>(); }
    template<class T> // Removes a component of type T from this entity
    inline bool Remove() { ComponentID index = _componentlist.Get(T::ID()); if(index != (ComponentID)~0) return T::Store().Remove(_componentlist[index]); }
    inline mgEntity* Parent() const { return _parent; }
    inline size_t NumChildren() const { return _children.Length(); }
    inline mgEntity* const* Children() const { return _children.begin(); }
    void SetParent(mgEntity* parent);
    inline int Order() const { return _order; }
    void SetOrder(int order);

    size_t id;
    size_t childhint; // Bitfield of scenegraph components our children might have
    size_t graphcomponents;

    static mgEntity& SceneGraph() { return _root; }
    static inline char Comp(mgEntity* const& l, mgEntity* const& r) { char c = SGNCOMPARE(l->_order, r->_order); return !c ? SGNCOMPARE(l, r) : c; }

  protected: 
    void _addchild(mgEntity* child);
    void _removechild(mgEntity* child);
    void _propagateIDs();

    bss_util::cMap<ComponentID, size_t, bss_util::CompT<ComponentID>, ComponentID> _componentlist; // You can't interact with componentlist directly because it violates DLL bounderies
    mgEntity* _parent;
    bss_util::cCompactArray<mgEntity*> _children; // each child takes a reference on its parent, preventing a parent from being destroyed until all it's children are gone.
    int _order;

    static mgEntity _root;
  };
}

#endif