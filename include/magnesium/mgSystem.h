// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __SYSTEM_H__MG__
#define __SYSTEM_H__MG__

#include "mgComponent.h"
#include "bss-util/ArraySort.h"

namespace magnesium {
  typedef unsigned short SystemID;
  typedef int MessageID;

  // A basic system implementation, doesn't iterate over anything
  class MG_DLLEXPORT mgSystemBase
  {
  public:
    explicit mgSystemBase(int priority = 0);
    mgSystemBase(mgSystemBase&& mov);
    virtual ~mgSystemBase();
    virtual void Process() = 0;
    virtual const char* GetName() const { return 0; }

    mgSystemBase& operator=(mgSystemBase&& mov);

    typedef void(*ITERATOR)(mgSystemBase*, mgEntity&);

  protected:
    friend class mgSystemManager;
    template<MessageID ID, typename R, typename... Args>
    friend struct MessageDef;

    virtual void _register() {}

    const int _priority;
    mgSystemManager* _manager;

  private:
    bss::ArraySort<MessageID> _activemsgs; // Managed by mgSystemManager
  };

  // This state is usually used for scripts, because it stores an arbitrary system processing state and doesn't use a hardcoded type ID
  class MG_DLLEXPORT mgSystemState : public mgSystemBase
  {
  public:
    mgSystemState() : _id(0), _name(0), _process(0) {}
    explicit mgSystemState(void(*process)(), const char* name, SystemID id, int priority = 0);
    mgSystemState(mgSystemState&& mov);
    virtual const char* GetName() const override { return _name; }
    virtual void Process() { _process(); }
    SystemID ID() const { return _id; }
    mgSystemState& operator=(mgSystemState&& mov);

  protected:
    void(*_process)();
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
      if (store != nullptr)
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
}

#endif