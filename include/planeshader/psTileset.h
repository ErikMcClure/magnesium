// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __TILESET_H__PS__
#define __TILESET_H__PS__

#include "psSolid.h"
#include "psDriver.h"
#include "psTextured.h"
#include "bss-util\cDynArray.h"

namespace planeshader {
  struct psTile
  {
    uint32_t index;
    float rotate;
    psVec pivot;
    uint32_t color;
  };

  class PS_DLLEXPORT psTileset : public psSolid, public psTextured, public psDriverHold
  {
    struct psTileDef {
      psRect uv;
      psRect rect;
      int level;
    };

  public:
    psTileset(const psTileset& copy);
    psTileset(psTileset&& mov);
    explicit psTileset(const psVec3D& position = VEC3D_ZERO, FNUM rotation = 0.0f, const psVec& pivot = VEC_ZERO, psFlag flags = 0, int zorder = 0, psStateblock* stateblock = 0, psShader* shader = 0, psPass* pass = 0, psInheritable* parent = 0, const psVec& scale = VEC_ONE);
    ~psTileset();
    inline psVeci GetTileDim() const { return _tiledim; }
    void SetTileDim(psVeci tiledim);
    uint32_t AutoGenDefs(psVec dim);
    uint32_t AddTileDef(psRect uv, psVec dim, psVec offset = VEC_ZERO, int level = 0);
    bool SetTile(psVeci pos, uint32_t index, uint32_t color = 0xFFFFFFFF, float rotate = 0, psVec pivot = VEC_ZERO);
    void SetTiles(psTile* tiles, uint32_t num, uint32_t pitch);
    inline psVeci GetDimIndex() const { return psVeci(_rowlength, _tiles.Length()/_rowlength); }
    void SetDimIndex(psVeci dim);

    virtual psTex* const* GetTextures() const override { return psTextured::GetTextures(); }
    virtual uint8_t NumTextures() const override { return psTextured::NumTextures(); }

    static inline uint32_t WangTile1D(uint32_t e1, uint32_t e2) {
      if(e1 < e2) return 2 * e1 + e2*e2;
      if(e1 == e2) return (e1 > 0) ? ((e1 + 1) * (e1 + 1) - 2) : 0;
      return (e2 > 0) ? (e1 * e1 + 2 * e2 - 1) : ((e1 + 1) * (e1 + 1) - 1);
    }
    static inline psVeciu WangTile2D(uint32_t e0, uint32_t e1, uint32_t e2, uint32_t e3) { return psVeciu(WangTile1D(e0, e2), WangTile1D(e1, e3)); }

    psTileset& operator=(const psTileset& copy);
    psTileset& operator=(psTileset&& mov);

  protected:
    virtual void _render() override;
    template<class T>
    BSS_FORCEINLINE bool _drawcheck(T* drawn, uint32_t k, int level) { return (k < _tiles.Length()) && !bss_util::bssGetBit<T>(drawn, k) && (level < _defs[_tiles[k].index].level); }
    
    uint32_t _rowlength;
    psVeci _tiledim; // Size of the actual tile for figuring out where to put each tile.
    bss_util::cDynArray<psTileDef, uint32_t> _defs; // For each tile indice, stores what the actual UV coordinates of that tile are and what the offset is
    bss_util::cDynArray<psTile, uint32_t> _tiles;
    bss_util::Matrix<float, 4, 4> _m;
  };
}
#endif