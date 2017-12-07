// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __COMPONENT_H__MG__
#define __COMPONENT_H__MG__

#include "mgEntity.h"

namespace magnesium {
  template<typename Component, typename... Components>
  inline void mgEntity_AddComponents(mgEntity* e) 
  { 
    e->Add<Component>();
    if constexpr(sizeof...(Components) > 0)
      mgEntity_AddComponents<Components...>(e); 
  }
  template<typename Component, typename... Components>
  inline void mgEntity_RemoveComponents(mgEntity* e)
  { 
    e->Remove<Component>(); 
    if constexpr(sizeof...(Components) > 0)
      mgEntity_RemoveComponents<Components...>(e);
  }

  class MG_DLLEXPORT mgComponentStoreBase
  {
  public:
    template<typename T>
    struct BSS_COMPILER_DLLEXPORT MagnesiumDLL {
      typedef T value_type;
      typedef void policy_type;
      template<class U> using rebind = MagnesiumDLL<U>;
      MagnesiumDLL() = default;
      template <class U> constexpr MagnesiumDLL(const MagnesiumDLL<U>&) noexcept {}

      inline T* allocate(size_t cnt, T* p = nullptr, size_t old = 0) noexcept { return reinterpret_cast<T*>(mgComponentStoreBase::dllrealloc(p, cnt * sizeof(T))); }
      inline void deallocate(T* p, size_t sz = 0) noexcept { mgComponentStoreBase::dllfree(p); }
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
    virtual mgComponentCounter* GetBase(size_t index) = 0;
    virtual size_t GetLength() const = 0;
    virtual void FlushBuffer() = 0;

    static mgComponentStoreBase* GetStore(ComponentID id);
    static bool RemoveComponent(ComponentID id, size_t index);
    static mgComponentCounter* GetComponent(ComponentID id, size_t index);
    static void* dllrealloc(void* p, size_t sz);
    static void dllfree(void* p);

    size_t curIteration;

  protected:
    const ComponentID _id;

  private:
    static bss::DynArray<mgComponentStoreBase*> _stores; // this must only be accessed inside the magnesium DLL
  };

  template<typename T, bss::ARRAY_TYPE ArrayType>
  class mgComponentStore : public mgComponentStoreBase
  {
  public:
    mgComponentStore() : mgComponentStoreBase(T::ID()) {}
    ~mgComponentStore() { Clear(); }
    template<typename D> // D is usually T, but sometimes it's a special constructor that contains additional information for the component
    inline size_t Add(mgEntity* p)
    {
      static_assert(sizeof(D) == sizeof(T), "Illegal alternative constructor");
      static_assert(std::is_base_of<T, D>::value, "Must be derived from T");
      assert(mgComponentRef<T>::Counter() <= 0);
      size_t index = reinterpret_cast<bss::DynArray<D, size_t, ArrayType, typename mgComponentStoreBase::MagnesiumDLL<D>>&>(_store).AddConstruct(p); // sneak in substitute constructor
      assert(_store.Back().entity == p);
      p->ComponentListInsert(_id, T::GraphID(), index);
      return index;
    }
    inline T* Get(size_t index = 0) { return (index < _store.Length()) ? _store.begin() + index : nullptr; }
    BSS_FORCEINLINE const T* begin() const noexcept { return _store.begin(); }
    BSS_FORCEINLINE const T* end() const noexcept { return _store.end(); }
    BSS_FORCEINLINE T* begin() noexcept { return _store.begin(); }
    BSS_FORCEINLINE T* end() noexcept { return _store.end(); }
    virtual mgEntity** GetEntity(size_t index) override { return (index < _store.Length()) ? &_store[index].entity : nullptr; }
    virtual mgComponentCounter* GetBase(size_t index) override { return (index < _store.Length()) ? &_store[index] : nullptr; }
    virtual size_t GetLength() const override { return _store.Length(); }
    virtual EntityIterator GetEntities() const override // By getting the address of the entity, this will work no matter what the inheritance structure of T is
    {
      return EntityIterator(const_cast<mgEntity**>(&_store.begin()->entity), _store.Length(), sizeof(T));
    }
    inline bool Remove(size_t index)
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
    inline void Clear() { while(RemoveInternal(_id, 0)); } // Remove all components by simply removing the root component until there are none left. This only takes O(n) time because the replacement operation is O(1)

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
    bss::DynArray<T, size_t, ArrayType, typename mgComponentStoreBase::MagnesiumDLL<T>> _store;
    bss::DynArray<size_t, size_t, bss::ARRAY_SIMPLE, typename mgComponentStoreBase::MagnesiumDLL<size_t>> _buf; // Buffered deletes
  };

  template<typename T, bool SCENEGRAPH = false, bss::ARRAY_TYPE ArrayType = bss::ARRAY_SIMPLE, typename... ImpliedComponents>
  struct mgComponent : mgComponentCounter
  {
    typedef T TYPE;
    static const int IMPLIED = sizeof...(ImpliedComponents);

    explicit mgComponent(mgEntity* e) : entity(e) { if constexpr(sizeof...(ImpliedComponents) > 0) mgEntity_AddComponents<ImpliedComponents...>(e); }
    mgComponent(const mgComponent& copy) : entity(copy.entity) {}
    mgComponent(mgComponent&& copy) : entity(copy.entity) {}
    mgComponent& operator=(const mgComponent& copy) { entity = copy.entity; return *this; }
    mgComponent& operator=(mgComponent&& copy) { entity = copy.entity; return *this; }

    mgEntity* entity;

    static ComponentID ID() { static ComponentID value = curID++; return value; }
    static ComponentID GraphID() { static ComponentID value = SCENEGRAPH ? ((curGraphID <<= 1) >> 1) : 0; return value; }
    static mgComponentStore<T, ArrayType>& Store() { static mgComponentStore<T, ArrayType> store; return store; }
    static void RemoveImplied(mgEntity* e) { if constexpr(sizeof...(ImpliedComponents) > 0) mgEntity_RemoveComponents<ImpliedComponents...>(e); }
  };

  template<typename T, bool SCENEGRAPH = false, bss::ARRAY_TYPE ArrayType = bss::ARRAY_SIMPLE>
  struct mgComponentInherit : mgComponent<mgComponentInherit<T,SCENEGRAPH,ArrayType>, SCENEGRAPH, ArrayType>
  {
    explicit mgComponentInherit(mgEntity* e, T* (*f)(mgEntity* e)) : mgComponent(e), func(f) {}
    inline T* Get() { return (*func)(entity); }
    inline const T* Get() const { return (*func)(entity); }
    T* (*func)(mgEntity* e);
  };

  template<typename T, typename D, bool SCENEGRAPH = false, bss::ARRAY_TYPE ArrayType = bss::ARRAY_SIMPLE>
  struct mgComponentInheritBind : mgComponentInherit<T, SCENEGRAPH, ArrayType>
  {
    explicit mgComponentInheritBind(mgEntity* e = 0) : mgComponentInherit(e, &CastComponent) {}
    static T* CastComponent(mgEntity* e) { return static_cast<T*>(e->Get<D>()); }
  };


  template<int A, int... Ax>
  struct add_variadic { static constexpr size_t value = A + add_variadic<Ax...>::value; };

  template<int A>
  struct add_variadic<A> { static constexpr size_t value = A; };

  template<typename... Components>
  struct mgEntityT : mgEntity
  {
    static const int NUMCOMPONENTS = sizeof...(Components)+add_variadic<Components::IMPLIED...>::value;

    explicit mgEntityT(mgEntity* parent = nullptr, int order = 0) : mgEntity(parent, order, NUMCOMPONENTS)
    {
      if constexpr(sizeof...(Components) > 0)
        mgEntity_AddComponents<Components...>(this);
    }
    virtual void DestroyThis() { delete this; }
  };

  MG_DLLEXPORT ComponentID GetComponentID(const char* name);
  MG_DLLEXPORT void RegisterComponentID(ComponentID id, const char* name);
  template<class T>
  inline void RegisterComponentID(const char* name) { RegisterComponentID(T::ID(), name); }
}

#endif