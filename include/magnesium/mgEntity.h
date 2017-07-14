// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __ENTITY_H__MG__
#define __ENTITY_H__MG__

#include "mgRefCounter.h"
#include "bss-util/Map.h"
#include "bss-util/CompactArray.h"
#include "bss-util/TRBTree.h"

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
    T& operator *() { return *ref; }
    bool operator !() const { return !ref; }
    const T& operator *() const { return *ref; }
    operator T*() { return ref; }

    static int& Counter() { static int count = 0; return count; }
    T* ref;
  };
#define COMPONENT_REF(T) mgComponentRef<T>
#else
#define COMPONENT_REF(T) T*
#endif

  struct MG_DLLEXPORT mgEntity : public mgRefCounter, public bss::internal::TRB_NodeBase<mgEntity>
  {
    explicit mgEntity(mgEntity* parent = 0, int order = 0);
    mgEntity(mgEntity&& mov);
    virtual ~mgEntity();
    void ComponentListInsert(ComponentID id, ComponentID graphid, size_t);
    void ComponentListRemove(ComponentID id, ComponentID graphid);
    size_t& ComponentListGet(ComponentID id);
    template<class T> // Gets a component of type T if it belongs to this entity, otherwise returns NULL
    inline COMPONENT_REF(T::template TYPE) Get() 
    { 
      ComponentID index = _componentlist.Get(T::ID()); 
      return index == (ComponentID)~0 ? nullptr : T::Store().Get(_componentlist[index]); 
    }
    template<class T> // Adds a component of type T to this entity if it doesn't already exist
    inline COMPONENT_REF(T::template TYPE) Add() 
    { 
      if(_componentlist.Get(T::ID()) == (ComponentID)~0)
        T::Store().Add<T>(this); 
      return Get<T::template TYPE>(); 
    }
    template<class T> // Removes a component of type T from this entity
    inline bool Remove() 
    { 
      ComponentID index = _componentlist.Get(T::ID()); 
      if(index != (ComponentID)~0) 
        return T::Store().Remove(_componentlist[index]);
      return false;
    }
    inline mgEntity* Parent() const { return _parent; }
    inline mgEntity* Children() const { return _first; }
    inline mgEntity* Next() const { return next; }
    void SetParent(mgEntity* parent);
    inline int Order() const { return _order; }
    void SetOrder(int order);
    const char* GetName() const { return _name; }
    void SetName(const char* name) { _name = name; }
    inline bss::Slice<const std::pair<ComponentID, size_t>> GetComponents() const {
      return bss::Slice<const std::pair<ComponentID, size_t>>(_componentlist.begin(), _componentlist.Length());
    }

    size_t id;
    size_t childhint; // Bitfield of scenegraph components our children might have
    size_t graphcomponents;

    static mgEntity& SceneGraph() { return root; }
    static inline char Comp(const mgEntity& l, const mgEntity& r) { char c = SGNCOMPARE(l._order, r._order); return !c ? SGNCOMPARE(&l, &r) : c; }

  protected:
    explicit mgEntity(bool isNIL);
    void _addchild(mgEntity* child);
    void _removechild(mgEntity* child);
    void _propagateIDs();

    bss::Map<ComponentID, size_t, bss::CompT<ComponentID>, ComponentID> _componentlist; // You can't interact with componentlist directly because it violates DLL bounderies
    mgEntity* _parent;
    mgEntity* _first;
    mgEntity* _last;
    mgEntity* _children;
    int _order;
    const char* _name;

    static mgEntity root;
    static mgEntity NIL;
  };
}

#endif