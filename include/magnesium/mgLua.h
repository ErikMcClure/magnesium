// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __LUA_H__MG__
#define __LUA_H__MG__

#include "mgSystem.h"
#include <istream>
#include "lua.hpp"

namespace magnesium {
  template<class T, int N>
  struct LuaStack;

  template<class T> // Integers
  struct LuaStack<T, 1>
  {
    static inline void Push(lua_State *L, const T& i) { lua_pushinteger(L, i); }
    static inline T Pop(lua_State *L) { T r = (T)lua_tointeger(L, -1); lua_pop(L, 1); return r; }
  };
  template<class T> // Pointers
  struct LuaStack<T, 2>
  {
    static inline void Push(lua_State *L, T p) { lua_pushlightuserdata(L, (void*)p); }
    static inline T Pop(lua_State *L) { T r = (T)lua_touserdata(L, -1); lua_pop(L, 1); return r; }
  };
  template<class T> // Floats
  struct LuaStack<T, 3>
  {
    static inline void Push(lua_State *L, T n) { lua_pushnumber(L, static_cast<lua_Number>(n)); }
    static inline T Pop(lua_State *L) { T r = static_cast<T>(lua_touserdata(L, -1)); lua_pop(L, 1); return r; }
  };
  template<> // Strings
  struct LuaStack<const char*, 0>
  {
    static inline void Push(lua_State *L, const char* s) { lua_pushstring(L, s); }
    static inline const char* Pop(lua_State *L) { const char* r = lua_tostring(L, -1); lua_pop(L, 1); return r; }
  };
  template<> // Strings
  struct LuaStack<std::string, 0>
  {
    static inline void Push(lua_State *L, std::string s) { lua_pushlstring(L, s.c_str(), s.size()); }
    static inline std::string Pop(lua_State *L) { size_t sz; const char* s = lua_tolstring(L, -1, &sz); std::string r(s, sz); lua_pop(L, 1); return r; }
  };
  template<> // Strings
  struct LuaStack<bss::Str, 0>
  {
    static inline void Push(lua_State *L, bss::Str s) { lua_pushlstring(L, s.c_str(), s.size()); }
    static inline bss::Str Pop(lua_State *L) { size_t sz; const char* s = lua_tolstring(L, -1, &sz); bss::Str r(s, sz); lua_pop(L, 1); return r; }
  };
  template<> // Boolean
  struct LuaStack<bool, 0>
  {
    static inline void Push(lua_State *L, bool b) { lua_pushboolean(L, b); }
    static inline bool Pop(lua_State *L) { bool r = lua_toboolean(L, -1); lua_pop(L, 1); return r; }
  };

  // A system that initializes and registers the Lua scripting system
  class MG_DLLEXPORT LuaSystem : public mgSystemBase
  {
    template<typename T>
    struct LS : std::integral_constant<int, 
      std::is_integral<T>::value + 
      (std::is_pointer<T>::value * 2) + 
      (std::is_floating_point<T>::value * 3)>
    {};

  public:
    LuaSystem(int priority = 0);
    ~LuaSystem();
    virtual void Process() override;
    int Load(std::istream& s, const char* name = 0);
    template<typename R, typename... Args>
    inline R CallLua(const char* function, Args... args) { return _callLua<R, sizeof...(Args), Args...>(function, args...); }
    inline int Print() { return lua_Print(_l); }

    static const int CHUNKSIZE = (1 << 16);

  protected:
    template<typename R, int N, typename Arg, typename... Args>
    inline R _callLua(const char* function, Arg arg, Args... args)
    {
      LuaStack<Arg, LS<Arg>::value>::Push(_l, arg);
      _callLua<R, N, Args...>(function, args...);
    }
    template<typename R, int N>
    inline R _callLua(const char* function)
    {
      lua_call(_l, N, 1);
      return LuaStack<R, LS<R>::value>::Pop(_l);
    }
    static const char* _luaStreamReader(lua_State *L, void *data, size_t *size);
    static int lua_Print(lua_State *L);

    lua_State* _l;
  };
}

extern "C" {
  struct _FG_ROOT;

  struct _FG_ROOT* lua_GetGUI();
  void* lua_GetEntityComponent(magnesium::mgEntity* entity, const char* name);
  int lua_RegisterSystem(void(*process)(), int priority, const char* name);
  void* lua_GetComponentStore(const char* name);

}
#endif