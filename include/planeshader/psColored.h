// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __COLORED_H__PS__
#define __COLORED_H__PS__

#include "ps_dec.h"
#include "psColor.h"

namespace planeshader {
  // Represents something that has color.
  class PS_DLLEXPORT psColored
  {
  public:
    psColored(const psColored& copy);
    explicit psColored(uint32_t color=0xFFFFFFFF);
    virtual ~psColored();
    inline const psColor32& GetColor() const { return _color; }
    virtual void SetColor(uint32_t color);
    inline void SetColor(uint8_t a, uint8_t r, uint8_t g, uint8_t b) { SetColor(psColor32(a,r,g,b)); }

    inline psColored& operator=(const psColored& right) { _color = right._color; return *this; }
    inline virtual psColored* Clone() const { return new psColored(*this); } // Clone function

    // Interpolation functions for animation
    //template<uint8_t TypeID>
    //static inline uint32_t colorinterpolate(const typename bss_util::AniAttributeT<TypeID>::TVT_ARRAY_T& rarr, bss_util::AniAttribute::IDTYPE index, double factor) {
    //  return psColor::Interpolate(rarr[index-1].value, rarr[index].value, factor);
    //}

  protected:
    void _setcolor(uint32_t color) { SetColor(color); }

    psColor32 _color;
  };
}


#endif