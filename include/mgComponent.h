// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __COMPONENT_H__MG__
#define __COMPONENT_H__MG__

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

  struct MG_DLLEXPORT mgEntity : public mgRefCounter, public bss_util::LLBase<mgEntity>
  {
    mgEntity();
    mgEntity(mgEntity&& mov);
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

    inline static mgEntity* GetEntityList() { return _entitylist; }

  protected: // You can't interact with componentlist directly because it violates DLL bounderies
    bss_util::cMap<ComponentID, size_t, bss_util::CompT<ComponentID>, ComponentID> _componentlist;

    static mgEntity* _entitylist;
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

    struct EntityIterator : public std::iterator<std::bidirectional_iterator_tag, mgEntity*> {
      inline EntityIterator(void* p_, size_t len_, size_t type_) : p(reinterpret_cast<uint8_t*>(p_)), end(reinterpret_cast<uint8_t*>(p_) + (len_*type_)), type(type_) {}
      inline mgEntity* operator*() const { return *reinterpret_cast<mgEntity**>(p); }
      inline EntityIterator& operator++() { p += type; return *this; } //prefix
      inline EntityIterator operator++(int) { EntityIterator r(*this); ++*this; return r; } //postfix
      inline EntityIterator& operator--() { p -= type; return *this; } //prefix
      inline EntityIterator operator--(int) { EntityIterator r(*this); --*this; return r; } //postfix
      inline bool operator==(const EntityIterator& _Right) const { return (p == _Right.p); }
      inline bool operator!=(const EntityIterator& _Right) const { return (p != _Right.p); }
      inline bool operator!() const { return !IsValid(); }
      inline operator bool() const { return IsValid(); }
      inline bool IsValid() const { return reinterpret_cast<size_t>(p) < reinterpret_cast<size_t>(end); }

      uint8_t* p;
      uint8_t* end;
      size_t type;
    };

    mgComponentStoreBase(ComponentID id);
    ~mgComponentStoreBase();
    virtual bool RemoveInternal(ComponentID id, size_t index) = 0; // we have to pass a component's own ID to it's remove function because this gets called from inside the DLL, which does not have access to the correct static instances
    virtual size_t MessageComponent(size_t index, void* msg, ptrdiff_t msgint) = 0;
    virtual EntityIterator GetEntities() const = 0;
    virtual mgEntity** GetEntity(size_t index) = 0;
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
    ~mgComponentStore() { Clear(); }
    size_t Add(mgEntity* p)
    { 
      size_t index = _store.AddConstruct(p); 
      //_store.Back().entity = p; 
      assert(_store.Back().entity == p);
      p->ComponentListInsert(_id, index);
      p->components += _id;
      return index;
    }
    T* Get(size_t index = 0) { return (index < _store.Length()) ? _store.begin() + index : nullptr; }
    virtual mgEntity** GetEntity(size_t index) override { return (index < _store.Length()) ? &_store[index].entity : nullptr; }
    virtual EntityIterator GetEntities() const override // By getting the address of the entity, this will work no matter what the inheritance structure of T is
    { 
      return EntityIterator(const_cast<mgEntity**>(&_store.begin()->entity), _store.Length(), sizeof(T));
    }
    bool Remove(size_t index)
    {
      if(index >= _store.Length()) return false;
      mgEntity* e = _store[index].entity;
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
    void Clear() { while(RemoveInternal(_id, 0)); } // Remove all components by simply removing the root component until there are none left. This only takes O(n) time because the replacement operation is O(1)

  protected:
    virtual bool RemoveInternal(ComponentID id, size_t index) override
    {
      if(index >= _store.Length()) return false;
      if(index < curIteration) // if we passed this during an iteration, we can't delete it yet.
      {
        _buf.Add(index);
        return true;
      }
      size_t last = _store.Length() - 1;
      if(index < last)
      {
        _store[index].~T();
        new (_store.begin() + index) T(std::move(_store[last]));
        //_store[index] = std::move(_store[last]);
        //_store[index].entity = _store[last].entity;
        assert(_store[index].entity == _store[last].entity);
        _store[index].entity->ComponentListGet(id) = index; // update the entity's tracked index
      }
      _store.RemoveLast();
      return true;
    }
    bss_util::cDynArray<T, size_t, ArrayType, typename mgComponentStoreBase::MagnesiumAllocPolicy<T>> _store;
    bss_util::cDynArray<size_t, size_t, bss_util::CARRAY_SIMPLE, typename mgComponentStoreBase::MagnesiumAllocPolicy<size_t>> _buf; // Buffered deletes
  };

  struct MG_DLLEXPORT mgComponentCounter { protected: static ComponentID curID; };

  template<typename T, bss_util::ARRAY_TYPE ArrayType = bss_util::CARRAY_SIMPLE>
  struct mgComponent : mgComponentCounter
  {
    explicit mgComponent(mgEntity* e) : entity(e) {}
    mgComponent(const mgComponent& copy) : entity(copy.entity) {}
    mgComponent(mgComponent&& copy) : entity(copy.entity) {}
    static ComponentID ID() { static ComponentID value = curID++; return value; }
    static mgComponentStore<T, ArrayType>& Store() { static mgComponentStore<T, ArrayType> store; return store; }
    size_t Message(void* msg, ptrdiff_t msgint) { return 0; } // Not virtual so we don't accidentally add a virtual function table to everything. Instead, this should be masked by the appropriate component, which will then get called by it's store.
    mgEntity* entity;

    mgComponent& operator=(const mgComponent& copy) { entity = copy.entity; return *this; }
    mgComponent& operator=(mgComponent&& copy) { entity = copy.entity; return *this; }
  };
}

#endif