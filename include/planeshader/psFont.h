// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __FONT_H__PS__
#define __FONT_H__PS__

#include "psTexFont.h"
#include "bss-util/cStr.h"

typedef struct FT_FaceRec_*  FT_Face;
typedef struct FT_LibraryRec_* FT_Library;

namespace planeshader {
  // Represents a font loaded from an actual Opentype/Truetype/etc. font definition. An absolute path can be specified, or it will look for fonts installed in the operating system.
  class PS_DLLEXPORT psFont : public psTexFont
  {
  public:
    // Iterates through the string and preloads all the glyphs that are used. This is useful for common characters that you know will be loaded eventually. Returns the number of characters successfully loaded 
    uint16_t PreloadGlyphs(const char* glyphs);
    uint16_t PreloadGlyphs(const int* glyphs);
    const psVeciu& GetDPI() const { return _dpi; }
    int GetPointSize() const { return _pointsize; }
    const char* GetPath() const { return _path; }

    static FT_Library PTRLIB;
    enum FONT_ANTIALIAS : uint8_t {
      FAA_NONE=0, //No antialiasing
      FAA_ANTIALIAS, //default antialiasing
      FAA_LCD, //antialiasing optimized for LCD displays
      FAA_LCD_V, //Same as above except for vertically oriented LCD displays
      FAA_LIGHT, //Antialiasing with a lighter technique
      FAA_SDF //Antialiasing using a Signed Distance Field. Requires using a shader that takes an SDF as input!
    };

    FONT_ANTIALIAS GetAntialias() const;
    static psFont* Create(const char* file, int psize, FONT_ANTIALIAS antialias = FAA_ANTIALIAS, const psVeciu& dpi = psVeciu(0, 0));
    static psFont* Create(const char* family, short weight, bool italic, int psize, FONT_ANTIALIAS antialias = FAA_ANTIALIAS, const psVeciu& dpi = psVeciu(0,0));

  protected:
    psFont(const psFont&) = delete;
    psFont(const char* file, int psize, FONT_ANTIALIAS antialias = FAA_ANTIALIAS, const psVeciu& dpi = psVeciu(0, 0));
    ~psFont();
    virtual psGlyph* _loadglyph(uint32_t codepoint) override;
    virtual float _loadkerning(uint32_t prev, uint32_t cur) override;
    psGlyph* _renderglyph(uint32_t codepoint);
    void _loadfont();
    void _cleanupfont();
    void _adjustantialias(FONT_ANTIALIAS antialiased);
    void _enforceantialias();
    void _stage();

    psFont& operator=(const psFont&) = delete;

    uint32_t _antialiased;
    int _pointsize;
    psVeciu _dpi;
    uint8_t* _buf;
    FT_Face _ft2face;
    psVeciu _curpos;
    uint32_t _nexty;
    uint8_t _curtex;
    cStr _path;
    cStr _hash;
    bss_util::cArray<psTex*, uint8_t> _staging;
    bool _haskerning;

    static bss_util::cHash<const char*, psFont*, true> _Fonts; //Hashlist of all fonts, done by file.
  };
}

#endif