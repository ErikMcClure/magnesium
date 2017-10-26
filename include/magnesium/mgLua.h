// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __LUA_H__MG__
#define __LUA_H__MG__

#include "mgSystem.h"
#include <istream>
#include "lua.hpp"

namespace magnesium {
  // A system that initializes and registers the Lua scripting system
  class MG_DLLEXPORT LuaSystem : public mgSystemBase
  {
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
    static mgComponentCounter* GetEntityComponent(mgEntity* e, const char* component);

    struct ProcessEvent {
      template<EventID ID, typename R, typename... Args>
      BSS_FORCEINLINE static void F(lua_State* L, mgEntity* e)
      {
        if constexpr(std::is_void<R>::value)
          Event<ID>::Send(e, (LuaPop<Args>(L))...);
        else
          LuaPush<R>(L, Event<ID>::Send(e, (LuaPop<Args>(L))...));
      }
    };
    struct ProcessMessage {
      template<EventID ID, typename R, typename... Args>
      BSS_FORCEINLINE static void F(lua_State* L, mgSystemManager* m)
      {
        if constexpr(std::is_void<R>::value)
          Message<ID>::Send(m, (LuaPop<Args>(L))...);
        else
          LuaPush<R>(L, Message<ID>::Send(m, (LuaPop<Args>(L))...));
      }
    };

    template<typename T>
    inline static void LuaPush(lua_State *L, const T& v)
    {
      if constexpr(std::is_same<T, bool>::value)
        lua_pushboolean(L, v);
      else if constexpr(std::is_base_of<std::string, T>::value)
        lua_pushlstring(L, v.c_str(), v.size());
      else if constexpr(std::is_same<typename std::remove_const<T>::type, char*>::value)
        lua_pushstring(L, v);
      else if constexpr(std::is_integral<T>::value || std::is_enum<T>::value)
        lua_pushinteger(L, static_cast<lua_Integer>(v));
      else if constexpr(std::is_pointer<T>::value || std::is_member_pointer<T>::value)
        lua_pushlightuserdata(L, (void*)v);
      else if constexpr(std::is_floating_point<T>::value)
        lua_pushnumber(L, static_cast<lua_Number>(v));
    }

    template<typename T>
    inline static T LuaPop(lua_State *L)
    {
      if constexpr(std::is_base_of<std::string, T>::value)
      {
        size_t sz;
        const char* s = lua_tolstring(L, -1, &sz);
        T r(s, sz);
        lua_pop(L, 1); 
        return r;
      }
      else
      {
        T r;

        if constexpr(std::is_same<T, bool>::value)
          r = lua_toboolean(L, -1);
        else if constexpr(std::is_integral<T>::value || std::is_enum<T>::value)
          r = static_cast<T>(lua_tointeger(L, -1));
        else if constexpr(std::is_pointer<T>::value || std::is_member_pointer<T>::value)
          r = (T)lua_touserdata(L, -1);
        else if constexpr(std::is_floating_point<T>::value)
          r = static_cast<T>(lua_tonumber(L, -1));

        lua_pop(L, 1); 
        return r;
      }
    }

  protected:
    template<typename R, int N, typename Arg, typename... Args>
    inline R _callLua(const char* function, Arg arg, Args... args)
    {
      LuaPush<Arg>(_l, arg);
      return _callLua<R, N, Args...>(function, args...);
    }
    template<typename R, int N>
    inline R _callLua(const char* function)
    {
      assert(!FPUsingle());
      lua_call(_l, N, std::is_void<R>::value ? 0 : 1);
      if constexpr(!std::is_void<R>::value)
        return LuaPop<R>(_l);
    }
    const char* _getError();
    void _popError();

    static const char* _luaStreamReader(lua_State *L, void *data, size_t *size);
    static int lua_Print(lua_State *L);
    static int _print(lua_State *L, std::ostream& out);

    lua_State* _l;
    bss::DynArray<mgSystemState, size_t, bss::ARRAY_SAFE> _systems;
  };
}

#endif