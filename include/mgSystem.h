// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __SYSTEM_H__MG__
#define __SYSTEM_H__MG__

#include "mgComponent.h"

namespace magnesium {
  // A basic system implementation, doesn't iterate over anything
  class MG_DLLEXPORT mgSystemBase
  {
  public:
    mgSystemBase(int priority = 0);
    virtual ~mgSystemBase();
    virtual void Process() = 0;

    //typedef unsigned short SystemID; // This doesn't work very well because most systems have singletons anyway
    //static SystemID sysid;
    //template<class T>
    //static SystemID ID() { static SystemID value = sysid++; return value; }

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
    virtual ~mgSystemSimple();
    virtual void Process();
    virtual void Iterate(mgEntity& entity) = 0;

  protected:
    const ComponentID _iterator;
  };

  // Iterates over the scene graph, ignoring any branches that do not have the required components.
  class MG_DLLEXPORT mgSystemComplex : public mgSystemBase
  {
  public:
    mgSystemComplex(size_t required, int priority = 0);
    virtual ~mgSystemComplex();
    virtual void Process();
    virtual void Iterate(mgEntity& entity) = 0;

  protected:
    void _process(mgEntity& root);

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
    void AddSystem(mgSystemBase* system);
    bool RemoveSystem(mgSystemBase* system);
    void Process();

    inline static char SortSystem(mgSystemBase* const& l, mgSystemBase* const& r) { char ret = SGNCOMPARE(l->_priority, r->_priority); return !ret ? SGNCOMPARE(l, r) : ret; }

  protected:
    bss_util::cArraySort<mgSystemBase*, &SortSystem> _systems;
  };
}

#endif