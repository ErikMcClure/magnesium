// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __TEXFONT_H__PS__
#define __TEXFONT_H__PS__

#include "psRect.h"
#include "psDriver.h"
#include "bss-util/cRefCounter.h"
#include "bss-util/cHash.h"
#include "bss-util/cArray.h"
#include "bss-util/delegate.h"

namespace planeshader {
  struct psGlyph
  {
    psRect uv; //uv coordinates to glyph on texture
    float bearingX; // This is added to the current cursor position for rendering
    float bearingY; // This is added to the current baseline
    float width; // This is the width of the glyph itself.
    float height; // This is the height of the glyph itself.
    float advance; // The cursor position is incremented by this much after rendering
    uint8_t texnum;
  };

  enum PSFONT_TEXTFLAGS : psFlag {
    PSFONT_RIGHT = (PSFLAG_USER << 0), // right-aligns the text. If the width is infinite this has no effect.
    PSFONT_CENTER = (PSFLAG_USER << 1), // horizontally centers the text.  If the width is infinite this has no effect
    PSFONT_BOTTOM = (PSFLAG_USER << 2), // bottom-aligns the text. If the height is infinite this has no effect
    PSFONT_VCENTER = (PSFLAG_USER << 3), // vertically centers the text. If the height is infinite this has no effect
    PSFONT_CLIP = (PSFLAG_USER << 4), // Clips characters outside of the rectangle.
    PSFONT_WORDBREAK = (PSFLAG_USER << 5), // Causes the text to break on words when the end of the rectangle is reached.
    PSFONT_CHARBREAK = (PSFLAG_USER << 6), // Causes the text to break on characters when the end of the rectangle is reached.
    PSFONT_PIXELSNAP = (PSFLAG_USER << 7), // Snaps all characters to integral coordinates to enhance drawing precision. Not recommended for moving text. Cannot compensate for the camera position.
    PSFONT_HARFBUZZ = (PSFLAG_USER << 8), // If possible, uses Harfbuzz for text shaping and advanced layout.
    PSFONT_RTL = (PSFLAG_USER << 9), // Forces right to left rendering
    PSFONT_CUSTOM = (PSFLAG_USER << 10) // Start custom flags here
  };

  // Represents any arbitrary font created out of a texture. This is used to implement psFont.
  class PS_DLLEXPORT psTexFont : public bss_util::cRefCounter, protected psDriverHold
  {
  public:
    typedef bss_util::delegate<void, size_t, psRectRotateZ&, uint32_t&> DELEGATE;
    // Draws text in the given rectangle. Returns the dimensions of the text which can be cached used to assemble an area if the text doesn't change.
    psVec DrawText(psShader* shader, const psStateblock* stateblock, const int* text, float lineheight, float letterspacing = 0.0f, const psRectRotateZ& area = psRectRotateZ(0, 0, 0, 0, 0), uint32_t color = 0xFFFFFFFF, psFlag flags = 0, DELEGATE d = DELEGATE(0, 0));
    psVec DrawText(psShader* shader, const psStateblock* stateblock, const char* text, float lineheight, float letterspacing = 0.0f, const psRectRotateZ& area = psRectRotateZ(0,0,0,0,0), uint32_t color = 0xFFFFFFFF, psFlag flags = 0, DELEGATE d = DELEGATE(0, 0));
    // Given the drawing flags and text, this calculates what size would be required to display all the text. If dest has nonnegative width or height, that dimension is kept constant while calculating the other.
    void CalcTextDim(const int* text, psVec& dest, float lineheight, float letterspacing = 0.0f, psFlag flags = 0);
    // Lets you access the underlying textures
    inline const psTex* GetTex(uint16_t index=0) const { assert(index<_textures.Capacity()); return _textures[index]; }
    // Adds a glyph definition
    void AddGlyph(int character, const psGlyph& glyph);
    // Adds a kerning amount
    void AddKerning(int prev, int character, float kerning = 0.0f);
    // Adds a texture
    uint8_t AddTexture(psTex* tex);
    virtual void DestroyThis() override;
    // Gets the default line height of the font
    inline float GetLineHeight() const noexcept { return _fontlineheight; }
    // Overrides the default line height
    inline void SetLineHeight(float lineheight) noexcept { _fontlineheight = lineheight; }
    std::pair<size_t, psVec> GetIndex(const int* text, float maxwidth, psFlag flags, float lineheight, float letterspacing, psVec pos);
    std::pair<size_t, psVec> GetPos(const int* text, float maxwidth, psFlag flags, float lineheight, float letterspacing, size_t index);

    static psTexFont* CreateTexFont(psTex* tex=0, float lineheight = 0.0f, float ascender = 0.0f, float descender = 0.0f);

  protected:
    psTexFont(const psTexFont& copy);
    psTexFont(psTexFont&& mov);
    psTexFont(psTex* tex, float lineheight, float ascender, float descender = 0.0f);
    psTexFont();
    virtual ~psTexFont();
    float _getlinewidth(const int*& text, float maxwidth, psFlag flags, float letterspacing, float& cur);
    BSS_FORCEINLINE psGlyph* _getchar(const int* text, float maxwidth, psFlag flags, float lineheight, float letterspacing, psVec& cursor, psRect& box, int& last, float& lastadvance, bool& dobreak);
    virtual psGlyph* _loadglyph(uint32_t codepoint);
    virtual float _loadkerning(uint32_t prev, uint32_t cur);
    float _getkerning(uint32_t prev, uint32_t cur);
    static bool _isspace(int c);

    bss_util::cHash<uint64_t, float> _kerning;
    bss_util::cHash<uint32_t, psGlyph> _glyphs;
    bss_util::cArray<psTex*, uint8_t> _textures;
    float _fontlineheight;
    float _fontascender;
    float _fontdescender;
  };
}

#endif