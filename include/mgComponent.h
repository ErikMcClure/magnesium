// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __COMPONENT_H__MG__
#define __COMPONENT_H__MG__

#include "mgRefCounter.h"
#include "bss-util/cMap.h"

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

  struct MG_DLLEXPORT mgEntity : public mgRefCounter
  {
    mgEntity();
    virtual ~mgEntity();
    void ComponentListInsert(ComponentID id, size_t);
    void ComponentListRemove(ComponentID id);
    size_t& ComponentListGet(ComponentID id);
    template<class T> // Gets a component of type T if it belongs to this entity, otherwise returns NULL
    T* Get() { ComponentID index = _componentlist.Get(T::ID()); return index == (ComponentID)~0 ? nullptr : T::Store().Get(_componentlist[index]); }
    template<class T> // Adds a component of type T to this entity
    T* Add() { T::Store().Add(this); return Get<T>(); }
    template<class T> // Removes a component of type T from this entity
    bool Remove() { ComponentID index = _componentlist.Get(T::ID()); if(index != (ComponentID)~0) return T::Store().Remove(_componentlist[index]); }

    size_t id;
    ComponentBitfield components; // bitfield of what components this entity has.

  protected: // You can't interact with componentlist directly because it violates DLL bounderies
    bss_util::cMap<ComponentID, size_t, bss_util::CompT<ComponentID>, ComponentID> _componentlist;
  };

  template<typename Component, typename... Components>
  struct mgEntity_AddComponents { static inline void f(mgEntity* e) { e->Add<Component>(); mgEntity_AddComponents<Components...>::f(e); } };

  template<typename Component>
  struct mgEntity_AddComponents<Component> { static inline void f(mgEntity* e) { e->Add<Component>(); } };

  template<typename... Components>
  struct mgEntityT : mgEntity { mgEntityT() : mgEntity() { mgEntity_AddComponents<Components...>::f(this); } virtual void DestroyThis() { delete this; } };

  class MG_DLLEXPORT mgComponentStoreBase
  {
  public:
    template<typename T>
    struct BSS_COMPILER_DLLEXPORT MagnesiumAllocPolicy {
      typedef T* pointer;
      typedef T value_type;
      template<typename U> struct rebind { typedef MagnesiumAllocPolicy<U> other; };

      inline static pointer allocate(size_t cnt, const pointer p = nullptr) noexcept { return reinterpret_cast<pointer>(mgComponentStoreBase::dllrealloc(p, cnt * sizeof(T))); }
      inline static void deallocate(pointer p, size_t = 0) noexcept { mgComponentStoreBase::dllfree(p); }
    };

    typedef bss_util::cDynArray<mgEntity*, size_t, bss_util::CARRAY_SIMPLE, MagnesiumAllocPolicy<mgEntity*>> ENTITY_ARRAY;

    mgComponentStoreBase(ComponentID id);
    ~mgComponentStoreBase();
    virtual bool RemoveInternal(ComponentID id, size_t index) = 0; // we have to pass a component's own ID to it's remove function because this gets called from inside the DLL, which does not have access to the correct static instances
    virtual size_t MessageComponent(size_t index, void* msg, ptrdiff_t msgint) = 0;
    virtual const ENTITY_ARRAY& GetEntityArray() const = 0;
    virtual void FlushBuffer() = 0;

    static mgComponentStoreBase* GetStore(ComponentID id);
    static bool RemoveComponent(ComponentID id, size_t index);
    static size_t MessageComponent(ComponentID id, size_t index, void* msg, ptrdiff_t msgint);
    static void* dllrealloc(void* p, size_t sz);
    static void dllfree(void* p);

    size_t curIteration;

  protected:
    const ComponentID _id;

  private:
    static bss_util::cDynArray<mgComponentStoreBase*> _stores; // this must only be accessed inside the magnesium DLL
  };

  template<typename T, bss_util::ARRAY_TYPE ArrayType>
  class mgComponentStore : protected mgComponentStoreBase
  {
  public:
    mgComponentStore() : mgComponentStoreBase(T::ID()) {}
    ~mgComponentStore() {
      while(RemoveInternal(_id, 0)); // Remove all components by simply removing the root component until there are none left. This only takes O(n) time because the replacement operation is O(1)
    }
    size_t Add(mgEntity* p)
    { 
      _refs.Add(p);
      size_t index = _store.AddConstruct(); 
      _store[index].Construct(p);
      p->ComponentListInsert(_id, index);
      p->components += _id;
      return index;
    }
    T* Get(size_t index = 0) { return (index < _store.Length()) ? _store.begin() + index : nullptr; }
    virtual const mgComponentStoreBase::ENTITY_ARRAY& GetEntityArray() const override { return _refs; }
    bool Remove(size_t index)
    {
      if(index >= _refs.Length()) return false;
      mgEntity* e = _refs[index];
      e->ComponentListRemove(_id);
      e->components -= _id;
      return RemoveInternal(_id, index);
    }
    virtual void FlushBuffer() override
    {
      curIteration = 0;
      for(size_t index : _buf)
        RemoveInternal(_id, index);
      _buf.Clear();
    }
    virtual size_t MessageComponent(size_t index, void* msg, ptrdiff_t msgint) override
    {
      if(index >= _store.Length()) return 0;
      return _store[index].Message(msg, msgint);
    }

  protected:
    virtual bool RemoveInternal(ComponentID id, size_t index) override
    {
      if(index >= _store.Length()) return false;
      if(index < curIteration) // if we passed this during an iteration, we can't delete it yet.
      {
        _buf.Add(index);
        return true;
      }
      size_t last = _refs.Length() - 1;
      if(index < last)
      {
        _refs[index] = _refs[last]; // Update the reference FIRST so the moved entity can find itself
        _store[index] = std::move(_store[last]);
        _refs[index]->ComponentListGet(id) = index; // update the entity's tracked index
      }
      _store.RemoveLast();
      _refs.RemoveLast();
      return true;
    }
    bss_util::cDynArray<T, size_t, ArrayType, typename mgComponentStoreBase::MagnesiumAllocPolicy<T>> _store;
    mgComponentStoreBase::ENTITY_ARRAY _refs;
    bss_util::cDynArray<size_t, size_t, bss_util::CARRAY_SIMPLE, typename mgComponentStoreBase::MagnesiumAllocPolicy<size_t>> _buf; // Buffered deletes
  };

  struct MG_DLLEXPORT mgComponentCounter { protected: static ComponentID curID; };

  template<typename T, bss_util::ARRAY_TYPE ArrayType = bss_util::CARRAY_SIMPLE>
  struct mgComponent : mgComponentCounter
  {
    static ComponentID ID() { static ComponentID value = curID++; return value; }
    static mgComponentStore<T, ArrayType>& Store() { static mgComponentStore<T, ArrayType> store; return store; }
    size_t Message(void* msg, ptrdiff_t msgint) { return 0; } // Not virtual so we don't accidentally add a virtual function table to everything. Instead, this should be masked by the appropriate component, which will then get called by it's store.
    void Construct(mgEntity* e) {}
  };
}

#endif