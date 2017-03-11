// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __SHADER_H__PS__
#define __SHADER_H__PS__

#include "psDriver.h"
#include "bss-util/cRefCounter.h"
#include <stdarg.h>

namespace planeshader {
  // Represents a single shader program of a certain type (pixel/vertex/etc.)
  struct SHADER_INFO : psDriverHold
  {
    // Directly creates a shader info object, which is not usually suggested but can be done if you have no constant data you care about.
    SHADER_INFO(void* Shader, SHADER_VER V, uint16_t sz=0, const void* Init=0) : shader(Shader), v(V), init(Init), ty_sz(sz) {}
    SHADER_INFO(const char* Shader, const char* entrypoint, SHADER_VER V, uint16_t sz=0, const void* Init=0) : shader(_driver->CompileShader(Shader, V, entrypoint)), v(V), init(Init), ty_sz(sz) {}
    void* shader;
    SHADER_VER v;
    uint16_t ty_sz;
    const void* init;

    // Creates a shader from either precompiled source, or a string with a given entrypoint, with optional initial data of type T stored as a constant buffer.
    template<class T> // We can't have templates on constructors so we have to do this instead
    BSS_FORCEINLINE static SHADER_INFO From(void* Shader, SHADER_VER V, const T* Init=0) { return SHADER_INFO(Shader, V, std::is_void<T>::value?0:sizeof(std::conditional<std::is_void<T>::value,char,T>::type), Init); }
    template<class T>
    BSS_FORCEINLINE static SHADER_INFO From(const char* Shader, const char* entrypoint, SHADER_VER V, const T* Init=0) { return SHADER_INFO(Shader, entrypoint, V, std::is_void<T>::value?0:sizeof(std::conditional<std::is_void<T>::value, char, T>::type), Init); }
  };

  // Encapsulates an entire shader effect: each type of shader program (vertex/pixel/etc.), the constants for each individual shader program, and the shader layout.
  class PS_DLLEXPORT psShader : protected psDriverHold, public bss_util::cRefCounter
  {
  public:
    void Activate() const;
    template<typename T, SHADER_VER I>
    inline bool SetConstants(const T& src)
    {
      uint8_t index = (I >= PIXEL_SHADER_1_1) + (I >= GEOMETRY_SHADER_4_0) + (I >= COMPUTE_SHADER_4_0) + (I >= DOMAIN_SHADER_5_0) + (I >= HULL_SHADER_5_0);
      return SetConstants(&src, sizeof(T), index);
    }
    bool SetConstants(const void* data, uint32_t sz, uint8_t I);

    // Creates a new shader object out of a given layout and a list of SHADER_INFOs, which represent all included shader programs and their associated constant buffers.
    template<uint8_t I>
    inline static psShader* CreateShader(const ELEMENT_DESC(&layout)[I], uint8_t num, ...)
    {
      DYNARRAY(SHADER_INFO, infos, num);
      va_list vl;
      va_start(vl, num);
      for(uint32_t i = 0; i < num; ++i) infos[i] = *va_arg(vl, const SHADER_INFO*);
      va_end(vl);
      return CreateShader(I, layout, num, infos);
    }
    static psShader* CreateShader(uint8_t nlayout, const ELEMENT_DESC* layout, uint8_t num, ...); // All arguments here must be passed in as const SHADER_INFO*
    template<uint8_t I>
    inline static psShader* CreateShader(const ELEMENT_DESC(&layout)[I], uint8_t num, const SHADER_INFO* infos) { return CreateShader(I, layout, num, infos); }
    static psShader* CreateShader(uint8_t nlayout, const ELEMENT_DESC* layout, uint8_t num, const SHADER_INFO* infos);
    // Copies a single shader
    static psShader* CreateShader(const psShader* copy);
    // merges num shaders into a new shader in left to right order (so the first will be overwritten by the rest). num cannot be 0.
    static psShader* MergeShaders(uint32_t num, const psShader* first, ...); 
    void** GetInternalPrograms() { return _ss; }

    psShader& operator+=(const psShader& right);

  protected:
    psShader(const psShader& copy);
    psShader(psShader&& mov);
    explicit psShader(void* layout, void* ss[6], void* sc[6], uint32_t sz[6]);
    ~psShader();
    void _destroy();
    void _copy(const psShader& copy);
    void _move(psShader&& mov);
    virtual void DestroyThis() override;
    psShader& operator=(const psShader& copy);
    psShader& operator=(psShader&& mov);

    void* _ss[6]; // Vertex, Pixel, Geometry, Compute, Domain, Hull
    void* _sc[6]; // Shader constants
    uint32_t _sz[6]; // Size of the shader constants (so we can copy the shader)
    void* _layout;
  };

  // This is just an imaginary wrapper around psShader that lets you do typesafe constant setting. It is completely optional.
  template<typename VS, typename PS=void, typename GS=void, typename CS=void, typename DS=void, typename HS=void>
  class PS_DLLEXPORT psShaderT : protected psShader
  {
  public:
    inline psShaderT(void* layout) : psShader<VS, PS, GS, CS, DS, HS>(layout)
    template<typename T> // Automatically picks the correct shader to apply the constants to by match T to that shader's constant declaration.
    inline bool SetConstants(const T& src)
    {
      static_assert((std::is_same<T, VS>::value+std::is_same<T, PS>::value+std::is_same<T, GS>::value+std::is_same<T, CS>::value+std::is_same<T, DS>::value+std::is_same<T, HS>::value) == 1);
      return SetConstants(&src, sizeof(T), (std::is_same<T, PS>::value*1)|(std::is_same<T, GS>::value*2)|(std::is_same<T, CS>::value*3)|(std::is_same<T, DS>::value*4)|(std::is_same<T, HS>::value*5));
    }

    template<uint8_t I>
    inline static psShaderT* CreateShader(const ELEMENT_DESC(&layout)[I], uint8_t num, ...) // All arguments here must be passed in as const SHADER_INFO*
    {
      DYNARRAY(SHADER_INFO, infos, num);
      va_list vl;
      va_start(vl, num);
      for(uint32_t i = 0; i < num; ++i) infos[i] = *va_arg(vl, const SHADER_INFO*);
      va_end(vl);
      return static_cast<psShaderT*>(psShader::CreateShader(I, layout, num, infos));
    }
    inline static psShaderT* CreateShader(uint8_t nlayout, const ELEMENT_DESC* layout, uint8_t num, ...) // All arguments here must be passed in as const SHADER_INFO*
    {
      DYNARRAY(SHADER_INFO, infos, num);
      va_list vl;
      va_start(vl, num);
      for(uint32_t i = 0; i < num; ++i) infos[i] = *va_arg(vl, const SHADER_INFO*);
      va_end(vl);
      return static_cast<psShaderT*>(psShader::CreateShader(nlayout, layout, num, infos));
    } 
    template<uint8_t I>
    inline static psShaderT* CreateShader(const ELEMENT_DESC(&layout)[I], uint8_t num, const SHADER_INFO* infos) { return static_cast<psShaderT*>(psShader::CreateShader(I, layout, num, infos)); }
    inline static psShaderT* CreateShader(uint8_t nlayout, const ELEMENT_DESC* layout, uint8_t num, const SHADER_INFO* infos) { return static_cast<psShaderT*>(psShader::CreateShader(layout, num, infos)); }
    inline static psShaderT* CreateShader(const psShaderT* copy) { return static_cast<psShaderT*>(psShader::CreateShader(copy)); }
  };
}

#endif