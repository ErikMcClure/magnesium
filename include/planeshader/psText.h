// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __TEXT_H__PS__
#define __TEXT_H__PS__

#include "psTexFont.h"
#include "psSolid.h"
#include "psColored.h"
#include "bss-util\cStr.h"
#include "bss-util\cArraySort.h"

namespace planeshader {
  class PS_DLLEXPORT psText : public psSolid, public psColored
  {
  public:
    psText(const psText& copy);
    psText(psText&& mov);
    explicit psText(psTexFont* font=0, const char* text = 0, const psVec3D& position = VEC3D_ZERO, FNUM rotation = 0.0f, const psVec& pivot = VEC_ZERO, psFlag flags = 0, int zorder = 0, psStateblock* stateblock = 0, psShader* shader = 0, psPass* pass = 0, psInheritable* parent = 0, const psVec& scale = VEC_ONE);
    ~psText();
    // Get/Set the text to be rendered
    inline const int* GetText() const { return _text; }
    inline void SetText(const char* text) { _text = text; _recalcdim(); }
    // Get/Set font used
    inline psTexFont* GetFont() const { return const_cast<psTexFont*>((const psTexFont*)_font); }
    void SetFont(psTexFont* font);
    // Get/Set the dimensions of the textbox - a dimension of 0 will grow to contain the text.
    inline psVec GetSize() const { return _textdim; }
    inline void SetSize(psVec size) { _textdim = size; _recalcdim(); }
    // Get/Set letter spacing
    inline float GetLetterSpacing() const { return _letterspacing; }
    inline void SetLetterSpacing(float spacing) { _letterspacing = spacing; _recalcdim(); }
    // Get/Set draw flags
    inline uint16_t GetDrawFlags() const { return _drawflags; }
    inline void SetDrawFlags(uint16_t flags) { _drawflags = flags; }
    // Get/Set per-character modification function
    inline const psTexFont::DELEGATE& GetFunc() const { return _func; }
    inline void SetFunc(psTexFont::DELEGATE func) { _func = func; }

  protected:
    virtual void BSS_FASTCALL _render() override;
    void _recalcdim();

    cStrT<int> _text;
    bss_util::cAutoRef<psTexFont> _font;
    psVec _textdim;
    float _letterspacing;
    uint16_t _drawflags;
    psTexFont::DELEGATE _func;
  };
}
#endif