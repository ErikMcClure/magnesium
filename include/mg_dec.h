// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __DEC_H__MG__
#define __DEC_H__MG__

#include "bss-util\bss_compiler.h"

#define MG_VERSION_MAJOR 0
#define MG_VERSION_MINOR 1
#define MG_VERSION_REVISION 0

#ifndef H_STATIC_LIB
#ifdef MAGNESIUM_EXPORTS
#pragma warning(disable:4251)
#define MG_DLLEXPORT BSS_COMPILER_DLLEXPORT
#else
#define MG_DLLEXPORT BSS_COMPILER_DLLIMPORT
#endif
#else
#define MG_DLLEXPORT
#endif

#endif
