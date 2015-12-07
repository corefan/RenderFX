/*
*	SSAO
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "MRenderFXEntry.h"
#include "MRenderSystem.h"

namespace Rad {

	class RFX_ENTRY SSAO
	{
	public:
		SSAO();
		~SSAO();

		void 
			Resize(int w, int h);
		void 
			Render(RenderTarget * rt, Texture * depthTex, Texture * normalTex);

	protected:
		void 
			_init();

		void 
			_renderAo(Texture * depthTex, Texture * normalTex);
		void 
			_blur();
		void 
			_blend(RenderTarget * rt);

	protected:
		RenderTargetPtr mRT_Quad0;
		RenderTargetPtr mRT_Quad1;

		TexturePtr mTex_Random;

		ShaderFX * mTech_Ao;
		ShaderFX * mTech_Blur;
		ShaderFX * mTech_Blend;
	};
}