/*
*	Ocean
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "MRenderFXEntry.h"
#include "MRenderSystem.h"
#include "MReflection.h"

namespace Rad {

	class ProjectedGrid;

	class RFX_ENTRY Ocean
	{
	public:
		Ocean();
		~Ocean();

		void
			Update(float frameTime);
		void 
			Render(Texture * depthTex, Texture * colorTex);
		void 
			Resize(int w, int h);

		void 
			SetHeight(float h) { mPosition.y = h; }
		const Float3 & 
			GetPosition() { return mPosition; }

		RenderContext *
			GetReflectionrContext() { return mRenderContext; }
		Texture *
			GetReflectionTexture() { return mRenderTarget->GetTexture().c_ptr(); }
		Texture *
			GetWaveTexture() { return mTex_Wave.c_ptr(); }

	protected:
		bool
			_isUnderWater();

		void 
			_renderUnderWater(Texture * depthTex, Texture * colorTex);
		void 
			_renderUpWater(Texture * depthTex, Texture * colorTex);

		void
			_renderClipPlane();

	protected:
		TexturePtr mTex_Wave;
		TexturePtr mTex_Fresnel;
		TexturePtr mTex_Normal0;
		TexturePtr mTex_Normal1;

		Camera * mCamera;
		RenderTargetPtr mRenderTarget;
		DepthBufferPtr mDepthBuffer;
		RenderPipelineReflection * mRenderPipeline;
		RenderContext * mRenderContext;

		ShaderFX * mTech;
		ShaderFX * mTech_UnderWater;

		Float3 mPosition;

		ProjectedGrid * mProjGrid;
	};

}