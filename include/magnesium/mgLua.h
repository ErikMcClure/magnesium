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
    static inline void Push(lua_State *L, T i) { lua_pushinteger(L, static_cast<lua_Integer>(i)); }
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
  struct LuaStack<bool, 1>
  {
    static inline void Push(lua_State *L, bool b) { lua_pushboolean(L, b); }
    static inline bool Pop(lua_State *L) { bool r = lua_toboolean(L, -1); lua_pop(L, 1); return r; }
  };
  template<> // Void return type
  struct LuaStack<void, 0> { static inline void Pop(lua_State *L) { } };

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
    int Load(std::istream& s, std::ostream& out);
    int Load(const char* script, std::ostream& out);
    int Load(const char* script);
    int Require(const char* name); // Equivelent to calling require("name") in Lua
    int AppendPath(const char* path); // Same as running: path .. ';' .. package.path
    SystemID RegisterSystem(void(*process)(), int priority, const char* name);
    template<typename R, typename... Args>
    inline R CallLua(const char* function, Args... args)
    {
      lua_getglobal(_l, function);
      return _callLua<R, sizeof...(Args), Args...>(function, args...);
    }
    inline int Print() { return lua_Print(_l); }
    inline int Print(std::ostream& out) { return _print(_l, out); }
    inline lua_State* GetState() { return _l; }
    void WriteError(int r, const char* name);
    virtual const char* GetName() const override { return "Lua"; }

    static const int CHUNKSIZE = (1 << 16);

  protected:
    template<typename R, int N, typename Arg, typename... Args>
    inline R _callLua(const char* function, Arg arg, Args... args)
    {
      LuaStack<Arg, LS<Arg>::value>::Push(_l, arg);
      return _callLua<R, N, Args...>(function, args...);
    }
    template<typename R, int N>
    inline R _callLua(const char* function)
    {
      assert(!FPUsingle());
      lua_call(_l, N, std::is_void<R>::value ? 0 : 1);
      return LuaStack<R, LS<R>::value>::Pop(_l);
    }
    const char* _getError();
    void _popError();

    static const char* _luaStreamReader(lua_State *L, void *data, size_t *size);
    static int lua_Print(lua_State *L);
    static int _print(lua_State *L, std::ostream& out);
    static int lua_GetEntityComponent(lua_State *L);

    lua_State* _l;
    bss::DynArray<mgSystemState, size_t, bss::ARRAY_SAFE> _systems;
  };
}

extern "C" {
  struct _FG_ROOT;

  struct _FG_ROOT* lua_GetGUI();
  int lua_RegisterSystem(void(*process)(), int priority, const char* name);
  magnesium::mgSystemBase::mgMessageResult lua_MessageSystem(const char* name, ptrdiff_t m, void* p);
}
#endif