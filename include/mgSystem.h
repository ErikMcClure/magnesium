// Copyright �2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __SYSTEM_H__MG__
#define __SYSTEM_H__MG__

#include "mgComponent.h"

namespace magnesium {
  class MG_DLLEXPORT mgSystemBase
  {
  public:
    mgSystemBase(ComponentBitfield required, int priority = 0);
    virtual ~mgSystemBase();
    virtual void Process() = 0;

    //typedef unsigned short SystemID; // This doesn't work very well because most systems have singletons anyway
    //static SystemID sysid;
    //template<class T>
    //static SystemID ID() { static SystemID value = sysid++; return value; }

  protected:
    friend class mgSystemManager;

    const int _priority;
    const ComponentBitfield _required;
    mgSystemManager* _manager;
  };

  class MG_DLLEXPORT mgSystemSimple : public mgSystemBase
  {
  public:
    mgSystemSimple(ComponentBitfield required, ComponentID iterator, int priority = 0);
    virtual ~mgSystemSimple();
    virtual void Process();
    virtual void Iterate(mgEntity* entity) = 0;

  protected:
    const ComponentID _iterator;
  };

  template<typename ComponentIterator, typename... Components>
  class BSS_COMPILER_DLLEXPORT mgSystem : public mgSystemSimple
  {
  public:
    mgSystem(int priority = 0) : mgSystemSimple(COMPONENTBITFIELD_EMPTY, ComponentIterator::ID(), priority) {}
  };

  template<typename... Components>
  class BSS_COMPILER_DLLEXPORT mgSystem<void, Components...> : public mgSystemBase
  {
  public:
    mgSystem(int priority = 0) : mgSystemBase(COMPONENTBITFIELD_EMPTY, priority) {}
  };

  class MG_DLLEXPORT mgSystemManager
  {
  public:
    mgSystemManager();
    ~mgSystemManager();
    void AddSystem(mgSystemBase* system);
    bool RemoveSystem(mgSystemBase* system);
    void Process();

    inline static char SortSystem(mgSystemBase* const& l, mgSystemBase* const& r) { char ret = SGNCOMPARE(l->_priority, r->_priority); return !ret ? SGNCOMPARE(l, r) : ret; }

  protected:
    bss_util::cArraySort<mgSystemBase*, &SortSystem> _systems;
  };
}

#endif