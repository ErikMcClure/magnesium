// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __SYSTEM_MANAGER_H__MG__
#define __SYSTEM_MANAGER_H__MG__

#include "mgSystem.h"

namespace magnesium {
  class MG_DLLEXPORT mgSystemManager
  {
  public:
    mgSystemManager();
    ~mgSystemManager();
    template<class T>
    inline void AddSystem(T* system) { AddSystem(system, GetSystemID<T>(system)); }
    void AddSystem(mgSystemBase* system, SystemID id);
    template<class T>
    inline bool RemoveSystem() { return RemoveSystem(GetSystemID<T>()); }
    bool RemoveSystem(SystemID id);
    SystemID RemoveSystem(mgSystemBase* system);
    template<class T>
    inline T* GetSystem() const { return static_cast<T*>(GetSystem(GetSystemID<T>())); }
    mgSystemBase* GetSystem(SystemID id) const;
    mgSystemBase* GetSystem(const char* name) const;
    inline bss::Slice<std::pair<mgSystemBase*, SystemID>> GetSystems() const { return _systems.GetSlice(); }
    void Process();
    template<typename F>
    inline void Defer(F && f) { _defer.push_back(std::forward<F>(f)); }
    void RunDeferred();
    inline static char SortSystem(mgSystemBase* const& l, mgSystemBase* const& r) { char ret = SGNCOMPARE(l->_priority, r->_priority); return !ret ? SGNCOMPARE(l, r) : ret; }

    template<class T>
    static SystemID GetSystemID() { static SystemID value = sysid++; return value; }
    template<class T>
    static inline SystemID GetSystemID(T* system)
    {
      if constexpr(std::is_base_of<mgSystemState, T>::value)
        return system->ID();
      else
        return GetSystemID<T>();
    }
    static SystemID GenerateSystemID() { return sysid++; }

    template<MessageID ID, typename R, typename... Args>
    friend struct MessageDef;

  protected:
    void _registerMessage(MessageID msg, SystemID id, void(*f)());
    template<typename R, typename... Args>
    inline R _sendMessage(MessageID ID, Args... args)
    {
      auto pair = _messagelist.GetValue(_messagelist.Iterator(ID));
      if(!pair)
        return R();
      mgSystemBase* base = _systemhash[pair->first];
      assert(base != 0);
      R(*f)(mgSystemBase*, Args...) = reinterpret_cast<R(*)(mgSystemBase*, Args...)>(pair->second);
      assert(f != 0);
      return f(base, args...);
    }

    static SystemID sysid;

    bss::Map<mgSystemBase*, SystemID, &SortSystem> _systems;
    bss::Hash<SystemID, mgSystemBase*> _systemhash;
    bss::Hash<const char*, mgSystemBase*> _systemname;
    bss::Hash<MessageID, std::pair<SystemID, void(*)()>> _messagelist;
    std::vector<std::function<void()>> _defer;
  };

  template<int I>
  struct Message;

  template<MessageID ID, typename R, typename... Args>
  struct MessageDef
  {
    BSS_FORCEINLINE static R Send(mgSystemManager* s, Args... args) { return s->_sendMessage<R, Args...>(ID, args...); }
    template<typename T>
    BSS_FORCEINLINE static void RegisterRaw(T* s, R(*f)(mgSystemBase*, Args...)) { s->_manager->_registerMessage(ID, mgSystemManager::GetSystemID<T>(s), reinterpret_cast<void(*)()>(f)); }
    template<typename T, R(T::*F)(Args...)>
    BSS_FORCEINLINE static void Register(T* s) { s->_manager->_registerMessage(ID, mgSystemManager::GetSystemID<T>(s), reinterpret_cast<void(*)()>(&stub<T, F>)); }
    template<typename T, typename... Inner>
    BSS_FORCEINLINE static auto Transfer(Inner... inner) { return T::F<ID, R, Args...>(inner...); }

  protected:
    template<typename T, R(T::*F)(Args...)>
    inline static R stub(mgSystemBase* p, Args... args) { return (static_cast<T*>(p)->*F)(args...); }
  };
}

#endif