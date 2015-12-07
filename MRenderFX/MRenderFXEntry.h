/*
*	RenderFX
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "MWorld.h"

#ifdef M_PLATFORM_WIN32

#ifdef RFX_EXPORT
#define RFX_ENTRY __declspec(dllexport)
#else
#define RFX_ENTRY __declspec(dllimport)
#endif

#else

#define RFX_ENTRY

#endif

