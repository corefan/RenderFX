/*
*	Shadow
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "MRenderFXEntry.h"
#include "MRenderSystem.h"

namespace Rad {

	class Shadow
	{
		friend class ShadowRenderPipline;
		friend class ShadowShaderProvider;

	public:
		static const int K_NumShadowLayers = 3;

		tEvent1<Camera *> E_VisibleCull;
		tEvent1<int> E_RenderDepth;

		struct CascadedView
		{
			Mat4 mView;
			Mat4 mProj;
			Mat4 mViewProj;
			Frustum mFrustum;
		};

	public:
		Shadow();
		virtual ~Shadow();

		void
			SetMapSize(int mapSize);
		int
			GetMapSize() { return mMapSize; }

		void
			SetDistance(int cascaded, float distance);
		float
			GetDistance(int cascaded) { return mDist[cascaded]; }

		void
			SetFadeRatio(float fadeRatio);
		float
			GetFadeRatio() { return mFadeRatio; }

		void
			SetOffset(float offset);
		float
			GetOffset() { return mOffset; }

		bool
			InVisible(int layer, const Aabb & bound);

		Camera *
			GetShadowCamera() { return mShadowCamera; }
		RenderContext *
			GetShadowContext() { return mShadowContext; }
		Texture *
			GetShadowTexture() { return mRT_Shadow->GetTexture().c_ptr(); }
		Texture *
			GetDepthTexture() { return mRT_Depth->GetTexture().c_ptr(); }
		CascadedView &
			GetCascadedView(int layer) { return mCascadedView[layer]; }

		void
			Resize(int w, int h) {}
		void
			Render(Texture * depthTex);

	protected:
		Mat4 
			_calcuCropMatrix(int layer);
		void 
			_calcuCascadedView(int layer);
		Aabb 
			_calcuAabb(const Float3 * v);
		void 
			_updateCamera();
		void 
			_applyShaderFX(RenderObject * able, int flag);
		void 
			_genShadowMap(int layer);

	protected:
		int mMapSize;
		float mDist[K_NumShadowLayers];
		float mBias[K_NumShadowLayers];
		float mOffset;
		float mFadeRatio;

		Texture * mDepthTexture;
		CascadedView mCascadedView[K_NumShadowLayers];
		Camera * mShadowCamera;
		RenderTargetPtr mRT_Depth;
		RenderTargetPtr mRT_Shadow;
		DepthBufferPtr mDepthBuffer;
		RenderContext * mShadowContext;

		ShaderFX * mTech_ShadowDepth;
		ShaderFX * mTech_ShadowDepthSkined;
		ShaderFX * mTech_Shadow[K_NumShadowLayers];
		TexturePtr mTex_Random;
	};

	//
	class ShadowVisibleCuller : public VisibleCuller
	{
	public:
		ShadowVisibleCuller(Shadow * shadow);
		virtual ~ShadowVisibleCuller() {}

		virtual void 
			DoCull(int frameId, Camera * camera);

	protected:
		Shadow * mShadow;
	};

	//
	class ShadowRenderPipline : public RenderPipeline
	{
	public:
		ShadowRenderPipline(Shadow * shadow);
		virtual ~ShadowRenderPipline() {}

		virtual void
			DoRender();

	protected:
		Shadow * mShadow;
	};

}