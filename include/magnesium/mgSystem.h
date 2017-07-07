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

    mgSystemBase(int priority = 0);
    virtual ~mgSystemBase();
    virtual void Process() = 0;
    virtual const char* GetName() const { return 0; }
    virtual mgMessageResult Message(ptrdiff_t m, void* p) { return mgMessageResult{ 0 }; }

    typedef void(*ITERATOR)(mgSystemBase*, mgEntity&);

  protected:
    friend class mgSystemManager;

    const int _priority;
    mgSystemManager* _manager;
  };

  // Iterates over the given component store
  class MG_DLLEXPORT mgSystemSimple : public mgSystemBase
  {
  public:
    mgSystemSimple(ComponentID iterator, int priority = 0);

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
    mgSystemComplex(size_t required, int priority = 0);

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

  template<typename ComponentIterator>
  class BSS_COMPILER_DLLEXPORT mgSystem : public mgSystemSimple
  {
  public:
    mgSystem(int priority = 0) : mgSystemSimple(ComponentIterator::ID(), priority) {}
  };

  template<>
  class BSS_COMPILER_DLLEXPORT mgSystem<void> : public mgSystemBase
  {
  public:
    mgSystem(int priority = 0) : mgSystemBase(priority) {}
  };

  class MG_DLLEXPORT mgSystemManager
  {
  public:
    mgSystemManager();
    ~mgSystemManager();
    template<class T>
    inline void AddSystem(T* system) { AddSystem(system, GetSystemID<T>()); }
    void AddSystem(mgSystemBase* system, SystemID id);
    template<class T>
    inline bool RemoveSystem() { return RemoveSystem(GetSystemID<T>()); }
    bool RemoveSystem(SystemID id);
    bool RemoveSystem(mgSystemBase* system);
    template<class T>
    inline T* GetSystem() const { return static_cast<T*>(GetSystem(GetSystemID<T>())); }
    mgSystemBase* GetSystem(SystemID id) const;
    mgSystemBase* GetSystem(const char* name) const;
    template<class T>
    inline mgSystemBase::mgMessageResult MessageSystem(ptrdiff_t m, void* p) { return MessageSystem(GetSystemID<T>(), m, p); }
    mgSystemBase::mgMessageResult MessageSystem(SystemID id, ptrdiff_t m, void* p);
    void Process();

    inline static char SortSystem(mgSystemBase* const& l, mgSystemBase* const& r) { char ret = SGNCOMPARE(l->_priority, r->_priority); return !ret ? SGNCOMPARE(l, r) : ret; }

    template<class T>
    static SystemID GetSystemID() { static SystemID value = sysid++; return value; }

  protected:
    static SystemID sysid;

    bss::Map<mgSystemBase*, SystemID, &SortSystem> _systems;
    bss::Hash<SystemID, mgSystemBase*> _systemhash;
    bss::Hash<const char*, mgSystemBase*> _systemname;
  };
}

#endif