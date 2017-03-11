// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __TEXTURED_H__PS__
#define __TEXTURED_H__PS__

#include "psTex.h"
#include "bss-util/cArray.h"
#include <vector>

namespace planeshader {
  // Represents any object that can be textured
  class PS_DLLEXPORT psTextured
  {
  public:
    psTextured(const psTextured& copy);
    psTextured(psTextured&& mov);
    explicit psTextured(const char* file);
    explicit psTextured(psTex* tex = 0);
    virtual ~psTextured();

    virtual void SetTexture(psTex* tex, uint32_t index = 0);
    void ClearTextures();
    inline const psTex* GetTexture(uint32_t index = 0) const { if(index>=_tex.Capacity()) return 0; return _tex[index]; }
    virtual inline psTex* const* GetTextures() const { return _tex; }
    virtual inline uint8_t NumTextures() const { return _tex.Capacity(); }
    inline virtual psTextured* Clone() const { return new psTextured(*this); } //Clone function

    psTextured& operator=(const psTextured& right);
    psTextured& operator=(psTextured&& right);

  protected:
    bss_util::cArray<psTex*, uint8_t> _tex;
  };
}

#endif