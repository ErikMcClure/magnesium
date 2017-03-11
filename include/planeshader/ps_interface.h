// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __INTERFACE_H__PS__
#define __INTERFACE_H__PS__

#include "ps_dec.h"

#ifdef  __cplusplus
using namespace planeshader;
extern "C" {
#endif

struct PSINIT;
struct psEngine;
struct psDriver;
struct psCamera;
struct psPass;
struct psLocatable;
struct psRenderable;
struct psShader;
struct psTex;
struct psStateblock;
struct psInheritable;
struct psSolid;
struct psTextured;
struct psColored;
struct psImage;

struct psVec {
  FNUM x;
  FNUM y;
};
struct psVeci {
  int x;
  int y;
};
struct psVeciu {
  uint32_t x;
  uint32_t y;
};
struct psVec3D {
  FNUM x;
  FNUM y;
  FNUM z;
};
struct psCircle {
  FNUM r;
  FNUM x;
  FNUM y;
};
struct psRect {
  FNUM left;
  FNUM top;
  FNUM right;
  FNUM bottom;
};
struct psRectl {
  long left;
  long top;
  long right;
  long bottom;
};

struct psColor {
  float r;
  float g;
  float b;
  float a;
};

struct psGUIEvent {
  union {
    struct { int x; int y; uint8_t button; uint8_t allbtn; }; // Mouse events
    struct { short scrolldelta; }; // Mouse scroll
    struct {  // Keys
      int keychar; //Only used by KEYCHAR, represents a utf32 character
      uint8_t keycode; //only used by KEYDOWN/KEYUP, represents an actual keycode, not a character
      char keydown;
      char sigkeys;
    };
    struct { float joyvalue; short joyaxis; }; // JOYAXIS
    struct { char joydown; short joybutton; }; // JOYBUTTON
  };
  uint8_t type;
  uint64_t time;
};

struct cEngine* cEngine_new(struct cEngine* ptr, const PSINIT* args);
void cEngine_delete(struct cEngine* ptr);



#ifdef  __cplusplus
}
#endif
#endif