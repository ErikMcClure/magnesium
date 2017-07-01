// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __COMPONENT_H__MG__
#define __COMPONENT_H__MG__

#include "mgEntity.h"

namespace magnesium {
  template<typename... Components>
  struct mgEntity_AddComponents { inline static void f(mgEntity* e) {} };
  template<typename Component, typename... Components>
  struct mgEntity_AddComponents<Component, Components...> { inline static void f(mgEntity* e) { e->Add<Component>(); mgEntity_AddComponents<Components...>::f(e); } };

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
    virtual EntityIterator GetEntities() const = 0;
    virtual mgEntity** GetEntity(size_t index) = 0;
    virtual void FlushBuffer() = 0;

    static mgComponentStoreBase* GetStore(ComponentID id);
    static bool RemoveComponent(ComponentID id, size_t index);
    static void* dllrealloc(void* p, size_t sz);
    static void dllfree(void* p);

    size_t curIteration;

  protected:
    const ComponentID _id;

  private:
    static bss::DynArray<mgComponentStoreBase*> _stores; // this must only be accessed inside the magnesium DLL
  };

  template<typename T, bss::ARRAY_TYPE ArrayType>
  class mgComponentStore : protected mgComponentStoreBase
  {
  public:
    mgComponentStore() : mgComponentStoreBase(T::ID()) {}
    ~mgComponentStore() { Clear(); }
    template<typename D> // D is usually T, but sometimes it's a special constructor that contains additional information for the component
    size_t Add(mgEntity* p)
    {
      static_assert(sizeof(D) == sizeof(T), "Illegal alternative constructor");
      static_assert(std::is_base_of<T, D>::value, "Must be derived from T");
      assert(mgComponentRef<T>::Counter() <= 0);
      size_t index = reinterpret_cast<bss::DynArray<D, size_t, ArrayType, typename mgComponentStoreBase::MagnesiumAllocPolicy<D>>&>(_store).AddConstruct(p); // sneak in substitute constructor
      assert(_store.Back().entity == p);
      p->ComponentListInsert(_id, T::GraphID(), index);
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
      e->ComponentListRemove(_id, T::GraphID());
      return RemoveInternal(_id, index);
    }
    virtual void FlushBuffer() override
    {
      curIteration = 0;
      for(size_t index : _buf)
        RemoveInternal(_id, index);
      _buf.Clear();
    }
    void Clear() { while(RemoveInternal(_id, 0)); } // Remove all components by simply removing the root component until there are none left. This only takes O(n) time because the replacement operation is O(1)

  protected:
    virtual bool RemoveInternal(ComponentID id, size_t index) override
    {
      assert(mgComponentRef<T>::Counter() <= 0);
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
    bss::DynArray<T, size_t, ArrayType, typename mgComponentStoreBase::MagnesiumAllocPolicy<T>> _store;
    bss::DynArray<size_t, size_t, bss::ARRAY_SIMPLE, typename mgComponentStoreBase::MagnesiumAllocPolicy<size_t>> _buf; // Buffered deletes
  };

  struct MG_DLLEXPORT mgComponentCounter { protected: static ComponentID curID; static ComponentID curGraphID; };

  template<typename T, bool SCENEGRAPH = false, bss::ARRAY_TYPE ArrayType = bss::ARRAY_SIMPLE, typename... ImpliedComponents>
  struct mgComponent : mgComponentCounter
  {
    explicit mgComponent(mgEntity* e) : entity(e) { mgEntity_AddComponents<ImpliedComponents...>::f(e); }
    mgComponent(const mgComponent& copy) : entity(copy.entity) {}
    mgComponent(mgComponent&& copy) : entity(copy.entity) {}
    static ComponentID ID() { static ComponentID value = curID++; return value; }
    static ComponentID GraphID() { static ComponentID value = SCENEGRAPH ? ((curGraphID <<= 1) >> 1) : 0; return value; }
    static mgComponentStore<T, ArrayType>& Store() { static mgComponentStore<T, ArrayType> store; return store; }
    mgEntity* entity;

    typedef T TYPE;
    mgComponent& operator=(const mgComponent& copy) { entity = copy.entity; return *this; }
    mgComponent& operator=(mgComponent&& copy) { entity = copy.entity; return *this; }
  };

  template<typename T, typename D, bool SCENEGRAPH = false, bss::ARRAY_TYPE ArrayType = bss::ARRAY_SIMPLE>
  struct mgComponentInherit : mgComponent<T, SCENEGRAPH, ArrayType>
  {
    explicit mgComponentInherit(mgEntity* e = 0, D* (*f)(mgEntity*) = 0) : mgComponent(e), func(f) {}
    D* (*func)(mgEntity*);
    D* Get() { return func(entity); }
  };

  template<class T, bool SCENEGRAPH>
  struct MG_DLLEXPORT mgComponentInheritBase : mgComponentInherit<mgComponentInheritBase<T, SCENEGRAPH>, T, SCENEGRAPH> {
    explicit mgComponentInheritBase(mgEntity* e = 0, T* (*f)(mgEntity*) = 0) : mgComponentInherit(e, f) {}
  };
  template<class D, class T, bool SCENEGRAPH> // Important: Remember that D should be the COMPONENT, not the object the component is representing!
  struct MG_DLLEXPORT mgComponentInheritInit : mgComponentInheritBase<T, SCENEGRAPH> {
    explicit mgComponentInheritInit(mgEntity* e = 0) : mgComponentInheritBase(e, &CastComponent) {}

    static inline T* CastComponent(mgEntity* entity) { return static_cast<T*>(entity->Get<D>()); } // Doing a direct cast here is preferred to an indirect method like Get() so null pointers are handled
  };

  struct MG_DLLEXPORT mgPosition : mgComponent<mgPosition>
  {
    float position[3];
    float rotation;
    float pivot[2];
  };
}

#endif