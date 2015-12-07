/*
*	EvParam
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "MRenderFXEntry.h"

namespace Rad {

	class RFX_ENTRY FXAA
	{
	public:
		FXAA();
		~FXAA();
		
		void
			Render(RenderTarget * rt, Texture * tex);

	protected:
		float mReduceMin;
		float mReduceMul;
		float mSpanMax;

		ShaderFX * mShaderFX;
	};

}