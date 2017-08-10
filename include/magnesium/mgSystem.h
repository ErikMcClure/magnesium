// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __SYSTEM_H__MG__
#define __SYSTEM_H__MG__

#include "mgComponent.h"

namespace magnesium {
  typedef unsigned short SystemID;

  // A basic system implementation, doesn't iterate over anything
  class MG_DLLEXPORT mgSystemBase
  {
  public:
    union mgMessageResult {
      size_t u;
      ptrdiff_t i;
      void* p;
      float f;
    };

    explicit mgSystemBase(int priority = 0);
    mgSystemBase(mgSystemBase&& mov);
    virtual ~mgSystemBase();
    virtual void Process() = 0;
    virtual const char* GetName() const { return 0; }
    virtual mgMessageResult Message(ptrdiff_t m, void* p) { return mgMessageResult{ 0 }; }

    mgSystemBase& operator=(mgSystemBase&& mov);

    typedef void(*ITERATOR)(mgSystemBase*, mgEntity&);

  protected:
    friend class mgSystemManager;

    const int _priority;
    mgSystemManager* _manager;
  };

  // This state is usually used for scripts, because it stores an arbitrary system processing state and doesn't use a hardcoded type ID
  class MG_DLLEXPORT mgSystemState : public mgSystemBase
  {
  public:
    mgSystemState() : _id(0), _name(0), _process(0), _message(0) {}
    explicit mgSystemState(void(*process)(), const char* name, SystemID id, int priority = 0, mgMessageResult(*message)(ptrdiff_t m, void* p) = DefaultMessageResult);
    mgSystemState(mgSystemState&& mov);
    virtual const char* GetName() const override { return _name; }
    virtual mgMessageResult Message(ptrdiff_t m, void* p) override { return _message(m, p); }
    virtual void Process() { _process(); }
    SystemID ID() const { return _id; }
    mgSystemState& operator=(mgSystemState&& mov);

    static inline mgMessageResult DefaultMessageResult(ptrdiff_t m, void* p) { return mgMessageResult{ 0 }; }

  protected:
    void(*_process)();
    mgMessageResult(*_message)(ptrdiff_t m, void* p);
    const char* _name;
    const SystemID _id;
  };

  // Iterates over the given component store
  class MG_DLLEXPORT mgSystemSimple : public mgSystemBase
  {
  public:
    explicit mgSystemSimple(ComponentID iterator, int priority = 0);
    mgSystemSimple(mgSystemSimple&& mov);

    mgSystemSimple& operator=(mgSystemSimple&& mov);

    template<ITERATOR F> // This allows generating your own iterator without using virtual function calls
    static void Iterate(mgSystemSimple* self) 
    {
      mgComponentStoreBase* store = mgComponentStoreBase::GetStore(self->_iterator);
      if (store != 0)
      {
        auto& entities = store->GetEntities();
        store->curIteration = 0;

        while (entities)
        {
          ++store->curIteration;
          F(self, **entities);
          ++entities;
        }
        store->FlushBuffer(); // Delete all the entities whose deletion was postponed because we had already iterated over them.
      }
    }

  protected:
    const ComponentID _iterator;
  };

  // Iterates over the scene graph, ignoring any branches that do not have the required components.
  class MG_DLLEXPORT mgSystemComplex : public mgSystemBase
  {
  public:
    explicit mgSystemComplex(size_t required, int priority = 0);
    mgSystemComplex(mgSystemComplex&& mov);

    mgSystemComplex& operator=(mgSystemComplex&& mov);

    template<ITERATOR F> // This allows generating your own iterator without using virtual function calls
    static void Iterate(mgSystemComplex* self, mgEntity& root)
    {
      int i = 0;
      size_t l = root.NumChildren();
      mgEntity* const* p = root.Children();

      if (root.childhint&self->_required)
      {
        while (i < l && p[i]->Order() < 0)
          Iterate<F>(*p[i++]);
      }
      if (root.graphcomponents&self->_required)
        F(self, root);
      if (root.childhint&self->_required)
      {
        while (i < l)
          Iterate<F>(*p[i++]);
      }
    }

  protected:
    const size_t _required;
  };

  template<typename SYS, typename COMP, typename... Args>
  struct SimpleIterator
  {
    // Efficient, type-specific iteration generator function
    template<void(SYS::*F)(COMP&, Args...)>
    BSS_FORCEINLINE static void Gen(SYS* self, Args... args)
    {
      auto& store = COMP::Store();
      store.curIteration = 0;

      for(COMP& c : store)
      {
        ++store.curIteration;
        (self->*F)(c, args...);
      }

      store.FlushBuffer(); // Delete all the entities whose deletion was postponed because we had already iterated over them.
    }
  };

  template<typename ComponentIterator>
  class BSS_COMPILER_DLLEXPORT mgSystem : public mgSystemSimple
  {
  public:
    explicit mgSystem(int priority = 0) : mgSystemSimple(ComponentIterator::ID(), priority) {}
    mgSystem(mgSystem&& mov) : mgSystemSimple(std::move(mov)) {}

    mgSystem& operator=(mgSystem&& mov) { mgSystemSimple::operator=(std::move(mov)); return *this; }
  };

  template<>
  class BSS_COMPILER_DLLEXPORT mgSystem<void> : public mgSystemBase
  {
  public:
    explicit mgSystem(int priority = 0) : mgSystemBase(priority) {}
    mgSystem(mgSystem&& mov) : mgSystemBase(std::move(mov)) {}

    mgSystem& operator=(mgSystem&& mov) { mgSystemBase::operator=(std::move(mov)); return *this; }
  };

  class MG_DLLEXPORT mgSystemManager
  {
  public:
    mgSystemManager();
    ~mgSystemManager();
    template<class T>
    inline void AddSystem(T* system) { AddSystem(system, GetSystemID<T>()); }
    void AddSystem(mgSystemBase* system, SystemID id);
    inline void AddSystemState(mgSystemState* system) { AddSystem(system, system->ID()); }
    template<class T>
    inline bool RemoveSystem() { return RemoveSystem(GetSystemID<T>()); }
    bool RemoveSystem(SystemID id);
    SystemID RemoveSystem(mgSystemBase* system);
    template<class T>
    inline T* GetSystem() const { return static_cast<T*>(GetSystem(GetSystemID<T>())); }
    mgSystemBase* GetSystem(SystemID id) const;
    mgSystemBase* GetSystem(const char* name) const;
    inline bss::Slice<std::pair<mgSystemBase*, SystemID>> GetSystems() const { return _systems.GetSlice(); }
    template<class T>
    inline mgSystemBase::mgMessageResult MessageSystem(ptrdiff_t m, void* p) { return MessageSystem(GetSystemID<T>(), m, p); }
    mgSystemBase::mgMessageResult MessageSystem(SystemID id, ptrdiff_t m, void* p);
    void Process();
    template<typename F>
    inline void Defer(F && f) { _defer.push_back(std::forward<F>(f)); }
    void RunDeferred();

    inline static char SortSystem(mgSystemBase* const& l, mgSystemBase* const& r) { char ret = SGNCOMPARE(l->_priority, r->_priority); return !ret ? SGNCOMPARE(l, r) : ret; }

    template<class T>
    static SystemID GetSystemID() { static SystemID value = sysid++; return value; }
    static SystemID GenerateSystemID() { return sysid++; }

  protected:
    static SystemID sysid;

    bss::Map<mgSystemBase*, SystemID, &SortSystem> _systems;
    bss::Hash<SystemID, mgSystemBase*> _systemhash;
    bss::Hash<const char*, mgSystemBase*> _systemname;
    std::vector<std::function<void()>> _defer;
  };
}

#endif