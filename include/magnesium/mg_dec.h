// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __DEC_H__MG__
#define __DEC_H__MG__

#define MG_VERSION_MAJOR 0
#define MG_VERSION_MINOR 1
#define MG_VERSION_REVISION 0

#ifndef _WINRESRC_

#include "bss-util\bss_compiler.h"

#if defined(MAGNESIUM_EXPORTS) || defined(MG_STATIC_LIB)
#pragma warning(disable:4251)
#define MG_DLLEXPORT BSS_COMPILER_DLLEXPORT
#else
#define MG_DLLEXPORT BSS_COMPILER_DLLIMPORT
#endif

#endif

#endif
