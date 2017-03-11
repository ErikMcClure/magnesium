// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __TEX_H__PS__
#define __TEX_H__PS__

#include "psDriver.h"
#include "psGUIManager.h"
#include "psStateblock.h"
#include "psColor.h"
#include "bss-util/cRefCounter.h"

namespace planeshader {
  class PS_DLLEXPORT psPixelArray : public psDriverHold
  {
    struct psPixelRow {
      struct psPixel {
        void* p;
        FORMATS fmt;
        inline psColor Color() const { psColor c; c.ReadFormat(fmt, p); return c; }
        inline psColor32 Color32() const { psColor32 c; c.ReadFormat(fmt, p); return c; }
        inline operator psColor() const { return Color(); }
        inline operator psColor32() const { return Color32(); }
        inline psPixel& operator=(const psColor& c) { c.WriteFormat(fmt, p); return *this; }
        inline psPixel& operator=(const psColor32& c) { c.WriteFormat(fmt, p); return *this; }
      };
      uint8_t* row;
      FORMATS fmt;

      inline psPixel operator[](uint32_t index) { return psPixel { row + (psColor::BitsPerPixel(fmt) >> 3)*index, fmt }; }
    };

  public:
    psPixelArray(void* res, FORMATS format, uint8_t lockflags = LOCK_READ, uint8_t miplevel = 0);
    ~psPixelArray();
    inline psPixelRow operator[](uint32_t index) { return psPixelRow { _mem + _rowpitch*index, _fmt }; }

  protected:
    uint8_t* _mem;
    void* _res;
    uint32_t _rowpitch;
    uint8_t _miplevel;
    FORMATS _fmt;
  };

  // Encapsulates an arbitrary texture not necessarily linked to an actual image
  class PS_DLLEXPORT psTex : public bss_util::cRefCounter, psDriverHold // The reference counter is optional
  {
  public:
    psTex(psTex&& mov);
    psTex(const psTex& copy);
    psTex(psVeciu dim, FORMATS format, uint32_t usage, uint8_t miplevels=0, psTexblock* texblock=0, psVeciu dpi = psVeciu(psGUIManager::BASE_DPI));
    psTex(void* res, void* view, psVeciu dim, FORMATS format, uint32_t usage, uint8_t miplevels, psTexblock* texblock=0, psVeciu dpi = psVeciu(psGUIManager::BASE_DPI)); // used to manually set res and view
    ~psTex();
    inline void* GetRes() const { return _res; }
    inline void* GetView() const { return _view; }
    inline psVec GetDim() const { return psVec(_dim)*(psVec((float)psGUIManager::BASE_DPI) / psVec(_dpi)); }
    inline const psVeciu& GetRawDim() const { return _dim; }
    inline uint8_t GetMipLevels() const { return _miplevels; }
    inline const psTexblock* GetTexblock() const { return _texblock; }
    inline void SetTexblock(psTexblock* texblock) { _texblock = texblock; }
    inline uint32_t GetUsage() const { return _usage; }
    inline FORMATS GetFormat() const { return _format; }
    inline void* Lock(uint32_t& rowpitch, psVeciu offset, uint8_t lockflags = LOCK_WRITE_DISCARD, uint8_t miplevel=0);
    inline void Unlock(uint8_t miplevel=0);
    inline psPixelArray LockPixels(uint8_t lockflags = LOCK_WRITE_DISCARD, uint8_t miplevel = 0) { return psPixelArray(_res, _format, lockflags, miplevel); }
    inline void SetDPI(psVeciu dpi) { _dpi = dpi; }
    inline psVeciu GetDPI() const { return _dpi; }

    // Attempts to resize the texture using the given method. Returns false if the attempt failed - if the attempt failed, the texture will not have been modified.
    enum RESIZE { RESIZE_DISCARD, RESIZE_CLIP, RESIZE_STRETCH };
    inline bool Resize(psVeciu dim, RESIZE resize = RESIZE_DISCARD);

    // Returns an existing texture object if it has the same path or creates a new one if necessary 
    static psTex* Create(const char* file, uint32_t usage = USAGE_SHADER_RESOURCE, FILTERS mipfilter = FILTER_TRIANGLE, uint8_t miplevels = 0, FILTERS loadfilter = FILTER_NONE, bool sRGB = false, psTexblock* texblock = 0, psVeciu dpi = psVeciu(psGUIManager::BASE_DPI));
    // if datasize is 0, data is assumed to be a path. If datasize is nonzero, data is assumed to be a pointer to memory where the texture is stored
    static psTex* Create(const void* data, uint32_t datasize, uint32_t usage = USAGE_SHADER_RESOURCE, FILTERS mipfilter = FILTER_TRIANGLE, uint8_t miplevels = 0, FILTERS loadfilter = FILTER_NONE, bool sRGB = false, psTexblock* texblock = 0, psVeciu dpi = psVeciu(psGUIManager::BASE_DPI));
    static psTex* Create(const psTex& copy);
    static psTex* Create(psVeciu dim, FORMATS format, uint32_t usage, uint8_t miplevels = 0, psTexblock* texblock = 0, psVeciu dpi = psVeciu(psGUIManager::BASE_DPI));

    psTex& operator=(const psTex& right);
    psTex& operator=(psTex&& right);

  protected:
    static psTex* _create(void* res, void* view, psVeciu dpi, psTexblock* texblock);

    void _applydesc(TEXTURE_DESC& desc);
    virtual void DestroyThis() override;

    void* _res; // In DX11 this is the shader resource view. In DX9 it's the texture pointer.
    void* _view; // In DX11 this is the render target or depth stencil view. In DX9 it's the surface pointer.
    psVeciu _dim; // Raw dimensions of the texture, not scaled by DPI
    uint8_t _miplevels;
    uint32_t _usage;
    FORMATS _format;
    bss_util::ref_ptr<psTexblock> _texblock;
    psVeciu _dpi; // Actual DPI of this texture. The returned dimensions are scaled by this.
  };
}

#endif