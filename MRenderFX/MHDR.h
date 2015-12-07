/*
*	HDR
*
*	Description: Implement Uncharted2 tone map
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "MRenderFXEntry.h"
#include "MRenderSystem.h"

namespace Rad {

	class RFX_ENTRY HDR
	{
	public:
		HDR();
		~HDR();

		void 
			Resize(int w, int h);

		void 
			Render(RenderTarget * rt, Texture * sceneTex, Texture * depthTex);

	protected:
		void 
			_initRT();

		void 
			_downScale(Texture * sceneTex);
		void 
			_lumInitial();
		void 
			_lumDownScale_16x();
		void 
			_lumDownScale_4x();
		void 
			_lumFinal();
		void 
			_lumAdapted();
		void 
			_bright();
		void 
			_blurH(RenderTarget * rt, Texture * tex);
		void 
			_blurV(RenderTarget * rt, Texture * tex);
		void 
			_final(RenderTarget * rt, Texture * sceneTex, Texture * depthTex);

	protected:
		RenderTargetPtr mRT_Quad;
		RenderTargetPtr mRT_Bloom0;
		RenderTargetPtr mRT_Bloom0_1;
		RenderTargetPtr mRT_Bloom1;
		RenderTargetPtr mRT_Bloom1_1;
		RenderTargetPtr mRT_Bloom2;
		RenderTargetPtr mRT_Bloom2_1;
		RenderTargetPtr mRT_64x64;
		RenderTargetPtr mRT_16x16;
		RenderTargetPtr mRT_4x4;
		RenderTargetPtr mRT_1x1;
		RenderTargetPtr mRT_1x1_0;
		RenderTargetPtr mRT_1x1_1;

		ShaderFX * mTech_DownScale2x;
		ShaderFX * mTech_DownScale4x;
		ShaderFX * mTech_LumInitial;
		ShaderFX * mTech_LumFinal;
		ShaderFX * mTech_Adapted;
		ShaderFX * mTech_Bright;
		ShaderFX * mTech_Blur;
		ShaderFX * mTech_ToneMap;
	};

}