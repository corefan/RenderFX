/*
*	GodRay
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "MRenderFXEntry.h"
#include "MRenderSystem.h"

namespace Rad {

	class RFX_ENTRY GodRay
	{
	public:
		GodRay();
		virtual ~GodRay();

		void 
			Resize(int w, int h);

		void 
			Render(RenderTarget * rt, Texture * depthTex);

	protected:
		void 
			_initGeometry();
		void 
			_initTechnique();
		void 
			_initRenderTarget();

		void 
			_occlusion();
		void 
			_renderSun();
		void 
			_godray(Texture * depthTex);
		void 
			_blur();
		void 
			_blend(RenderTarget * rt);

	protected:
		RenderOp mRenderOp;

		ShaderFX * mTech_SunOcc;
		ShaderFX * mTech_Sun;
		ShaderFX * mTech_GodRay;
		ShaderFX * mTech_Blur;
		ShaderFX * mTech_Blend;

		RenderTargetPtr mRenderTarget;
		RenderTargetPtr mRenderTarget1;
		float mOccLighting;
		float mOccRadius;
	};
}