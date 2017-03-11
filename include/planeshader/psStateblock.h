// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __STATEBLOCK_H__PS__
#define __STATEBLOCK_H__PS__

#include "ps_dec.h"
#include "bss-util/cHash.h"
#include "bss-util/cArray.h"
#include "bss-util/cRefCounter.h"

namespace planeshader {
  enum STATETYPE : uint8_t {
    TYPE_BLEND_ALPHATOCOVERAGEENABLE=0,
    TYPE_BLEND_INDEPENDENTBLENDENABLE,
    TYPE_BLEND_BLENDENABLE, // index 0-7
    TYPE_BLEND_SRCBLEND, // index 0-7 if INDEPENDENTBLENDENABLE is supported and enabled
    TYPE_BLEND_DESTBLEND, // see above
    TYPE_BLEND_BLENDOP, // see above
    TYPE_BLEND_SRCBLENDALPHA, // see above
    TYPE_BLEND_DESTBLENDALPHA, // see above
    TYPE_BLEND_BLENDOPALPHA, // see above
    TYPE_BLEND_RENDERTARGETWRITEMASK, // index 0-7
    TYPE_BLEND_BLENDFACTOR, // index 0-3
    TYPE_BLEND_SAMPLEMASK,
    TYPE_DEPTH_DEPTHENABLE,
    TYPE_DEPTH_DEPTHWRITEMASK,
    TYPE_DEPTH_DEPTHFUNC,
    TYPE_DEPTH_STENCILVALUE, // StencilRef value everything is compared to
    TYPE_DEPTH_STENCILENABLE,
    TYPE_DEPTH_STENCILREADMASK,
    TYPE_DEPTH_STENCILWRITEMASK,
    TYPE_DEPTH_FRONT_STENCILFAILOP,
    TYPE_DEPTH_FRONT_STENCILDEPTHFAILOP,
    TYPE_DEPTH_FRONT_STENCILPASSOP,
    TYPE_DEPTH_FRONT_STENCILFUNC,
    TYPE_DEPTH_BACK_STENCILFAILOP,
    TYPE_DEPTH_BACK_STENCILDEPTHFAILOP,
    TYPE_DEPTH_BACK_STENCILPASSOP,
    TYPE_DEPTH_BACK_STENCILFUNC,
    TYPE_RASTER_FILLMODE,
    TYPE_RASTER_CULLMODE,
    TYPE_RASTER_FRONTCOUNTERCLOCKWISE,
    TYPE_RASTER_DEPTHBIAS,
    TYPE_RASTER_DEPTHBIASCLAMP,
    TYPE_RASTER_SLOPESCALEDDEPTHBIAS,
    TYPE_RASTER_DEPTHCLIPENABLE,
    TYPE_RASTER_SCISSORENABLE,
    TYPE_RASTER_MULTISAMPLEENABLE,
    TYPE_RASTER_ANTIALIASEDLINEENABLE,
    TYPE_SAMPLER_FILTER, // all samplerstates have index 0-15 for PS, 16-31 for VS, 32-47 for GS, and 48-63 for CS
    TYPE_SAMPLER_ADDRESSU,
    TYPE_SAMPLER_ADDRESSV,
    TYPE_SAMPLER_ADDRESSW,
    TYPE_SAMPLER_MIPLODBIAS,
    TYPE_SAMPLER_MAXANISOTROPY,
    TYPE_SAMPLER_COMPARISONFUNC,
    TYPE_SAMPLER_BORDERCOLOR1,
    TYPE_SAMPLER_BORDERCOLOR2,
    TYPE_SAMPLER_BORDERCOLOR3,
    TYPE_SAMPLER_BORDERCOLOR4,
    TYPE_SAMPLER_MINLOD,
    TYPE_SAMPLER_MAXLOD,
    TYPE_TEXTURE_COLOROP, //legacy support for fixed-function texture blending, index 0-7
    TYPE_TEXTURE_COLORARG1,
    TYPE_TEXTURE_COLORARG2,
    TYPE_TEXTURE_ALPHAOP,
    TYPE_TEXTURE_ALPHAARG1,
    TYPE_TEXTURE_ALPHAARG2,
    TYPE_TEXTURE_TEXCOORDINDEX,
    TYPE_TEXTURE_COLORARG0,
    TYPE_TEXTURE_ALPHAARG0,
    TYPE_TEXTURE_RESULTARG,
    TYPE_TEXTURE_CONSTANT,
  };

  struct STATEINFO
  {
    STATEINFO(STATETYPE t, uint32_t v, uint16_t i = 0) : value(v), type(t), index(i) {}
    STATEINFO(STATETYPE t, uint16_t i, float f) : valuef(f), type(t), index(i) {}
    union {
      struct {
        union {
          uint32_t value;
          float valuef;
        };
        STATETYPE type;
        uint16_t index;
      };
      uint64_t __vali64;
    };

    typedef bss_util::cArray<STATEINFO, uint16_t> STATEINFOS;

    inline static bool SILESS(const planeshader::STATEINFO& l, const planeshader::STATEINFO& r) { return l.type<=r.type && (l.type<r.type || l.index<r.index); }
    inline static khint_t SIHASHFUNC(STATEINFOS* const& sb) {
      uint16_t sz=sb->Capacity();
      khint32_t r=0;
      for(uint16_t i = 0; i < sz; ++i)
        r=bss_util::KH_INT64_HASHFUNC((((int64_t)bss_util::KH_INT64_HASHFUNC((*sb)[i].__vali64))<<32)|r);
      return r;
    }
    inline static bool SIEQUALITY(STATEINFOS* const& left, STATEINFOS* const& right)
    {
      uint16_t sl=left->Capacity();
      uint16_t sr=right->Capacity();
      if(sl!=sr) return false;
      for(uint16_t i = 0; i < sl; ++i)
        if((*left)[i].__vali64 != (*right)[i].__vali64)
          return false;
      return true;
    }
    static STATEINFOS* Exists(STATEINFOS* compare);
    typedef bss_util::cHashBase<STATEINFOS*, char, false, &SIHASHFUNC, &SIEQUALITY> BLOCKHASH;
    static BLOCKHASH _blocks;
  };

  class PS_DLLEXPORT psStateblock : public bss_util::cArray<STATEINFO, uint16_t>, public bss_util::cRefCounter
  {
  public:
    inline void* GetSB() const { return _sb; }
    psStateblock* Combine(psStateblock* other) const;
    psStateblock* Append(STATEINFO state) const;

    static psStateblock* Create(uint32_t numstates, ...);
    static psStateblock* Create(const STATEINFO* infos, uint32_t numstates);

    static psStateblock* DEFAULT;

  protected:
    psStateblock(const STATEINFO* infos, uint32_t numstates);
    ~psStateblock();
    virtual void DestroyThis() override;

    void* _sb;
  };

  // Restricted to sampler infos
  class PS_DLLEXPORT psTexblock : public bss_util::cArray<STATEINFO, uint16_t>, public bss_util::cRefCounter
  {
  public:
    inline void* GetSB() const { return _tb; }
    psTexblock* Combine(psTexblock* other) const;
    psTexblock* Append(STATEINFO state) const;

    static psTexblock* Create(uint32_t numstates, ...);
    static psTexblock* Create(const STATEINFO* infos, uint32_t numstates);

    static psTexblock* DEFAULT;

  protected:
    psTexblock(const STATEINFO* infos, uint32_t numstates);
    ~psTexblock();
    virtual void DestroyThis() override;

    void* _tb; // In DX11, this is the sampler state, but in openGL, it's actually NULL, because openGL binds the sampler states to the texture at texture creation time using the STATEINFOs contained in this object.
  };

  struct PS_DLLEXPORT STATEBLOCK_LIBRARY
  {
    static void INITLIBRARY();
    static psStateblock* GLOW; //Basic glow
    static psTexblock* UVBORDER; // Sets UV coordinates to use a border of color 0 (you can easily override that)
    static psTexblock* UVMIRROR; // Sets UV coordinates to Mirror
    static psTexblock* UVMIRRORONCE; // Sets UV coordinates to MirrorOnce
    static psTexblock* UVWRAP; // Sets UV coordinates to Wrap
    static psTexblock* UVCLAMP; // Sets UV coordinates to Clamp
    static psTexblock* POINTSAMPLE; // uses point sampling instead of linear sampling. Used for shaders that require precise pixel sampling.
    static psStateblock* REPLACE; // discards the destination pixels entirely.
    static psStateblock* SUBPIXELBLEND; // Treats each color channel as an alpha channel. Used for subpixel hinted text.
    static psStateblock* SUBPIXELBLEND1; // Same as above, but uses SRC1_COLOR, which requires a pixel shader that outputs to both SV_TARGET0 and SV_TARGET1
    static psStateblock* PREMULTIPLIED; // Blending state for premultiplied textures
  };

  enum STATEBLEND
  {
    STATEBLEND_ZERO = 1,
    STATEBLEND_ONE = 2,
    STATEBLEND_SRC_COLOR = 3,
    STATEBLEND_INV_SRC_COLOR = 4,
    STATEBLEND_SRC_ALPHA = 5,
    STATEBLEND_INV_SRC_ALPHA = 6,
    STATEBLEND_DEST_ALPHA = 7,
    STATEBLEND_INV_DEST_ALPHA = 8,
    STATEBLEND_DEST_COLOR = 9,
    STATEBLEND_INV_DEST_COLOR = 10,
    STATEBLEND_SRC_ALPHA_SAT = 11,
    STATEBLEND_BLEND_FACTOR = 14,
    STATEBLEND_INV_BLEND_FACTOR = 15,
    STATEBLEND_SRC1_COLOR = 16,
    STATEBLEND_INV_SRC1_COLOR = 17,
    STATEBLEND_SRC1_ALPHA = 18,
    STATEBLEND_INV_SRC1_ALPHA = 19
  };

  enum STATEBLENDOP
  {
    STATEBLENDOP_ADD = 1,
    STATEBLENDOP_SUBTRACT = 2,
    STATEBLENDOP_REV_SUBTRACT = 3,
    STATEBLENDOP_MIN = 4,
    STATEBLENDOP_MAX = 5
  };

  enum STATETEXTUREADDRESS
  {
    STATETEXTUREADDRESS_WRAP = 1,
    STATETEXTUREADDRESS_MIRROR = 2,
    STATETEXTUREADDRESS_CLAMP = 3,
    STATETEXTUREADDRESS_BORDER = 4,
    STATETEXTUREADDRESS_MIRROR_ONCE = 5
  };
}

#endif