/*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "Plugin.h"

class RenderFXPlugin : public Plugin
{
public:
	virtual ~RenderFXPlugin();

	virtual void 
		Init();
	virtual void 
		Shutdown();
};

