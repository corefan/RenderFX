/*
*	RenderFX
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "MRenderFXEntry.h"
#include "MSingleton.h"
#include "MEvParam.h"
#include "MSky.h"
#include "MSun.h"
#include "MRain.h"
#include "MSnow.h"
#include "MStarfield.h"
#include "MCloud.h"
#include "MOcean.h"
#include "MHDR.h"
#include "MGodRay.h"
#include "MSSAO.h"
#include "MShadow.h"
#include "MLighting.h"
#include "MFXAA.h"

namespace Rad {

#define RFX_CONTETX_ORDER 1024
#define RFX_SPECIAL_SAMPLER_LIGHTING 9

	class RFX_ENTRY RenderFX : public Singleton<RenderFX>
	{
	public:
		tEvent2<int, int> E_Resize;

		tEvent0 E_RenderGBufferBegin;
		tEvent0 E_RenderGBufferEnd;

	public:
		RenderFX();
		~RenderFX();

		void 
			Load(const String & filename);
		void 
			Save(const String & filename);

		RenderContext *
			GetContext() { return mContext; }
		RenderTargetPtr
			GetColorBuffer() { return mColorBuffer; }
		ShaderFX *
			GetClearFX() { return mClearFX; }

		EvKeyFrame * 
			GetEvKeyFrame(int type) { return &mKeyFrames[type]; }
		EvGlobalParam *
			GetEvGlobalParam() { return &mGlobalParam; }
		const EvParam * 
			GetEvParam() { return &mParam; }

		void 
			SetKeyType(eEvKeyType type);
		eEvKeyType 
			GetKeyType() { return mCurrentKey; }
		float 
			GetKeyTime(eEvKeyType type);
		const EvKeyFrame &
			GetCurrentKeyFrame() { return mKeyFrameCurrent; }

		void
			FadeIn(eEvKeyType type, float time);
		void
			FadeOut(float time);

		void 
			Update(float frameTime);
		void
			Resize(int w, int h);

	protected:
		float 
			_getSkyU(float time);
		float 
			_getSunRoll(float time);
		float 
			_getMoonRoll(float time);
		float 
			_getLightRoll(float time);

		Float3
			_makeSunDir(float y, float p, float r);
		Float3 
			_makeMoonDir(float y, float p, float r);
		Float3 
			_makeLightDir(const Float3 & sunDir, const Float3 & moonDir);

		void
			OnSceneLoad();
		void
			OnSceneUnload();

		void
			_updateColorTexture();
		void
			OnRenderSolid();
		void 
			OnRenderTransparent();
		void
			OnRenderConextEnd();
		void
			OnRenderEnd();

	protected:
		RenderContext * mContext;
		RenderTargetPtr mColorBuffer;
		RenderTargetPtr mColorBufferEx;
		ShaderFX * mClearFX;

		Sky * mSky;
		Sun * mSun;
		Rain * mRain;
		Snow * mSnow;
		Starfield * mStarfield;
		Cloud * mCloud;
		Ocean * mOcean;

		HDR * mHDR;
		GodRay * mGodRay;
		SSAO * mSSAO;
		Shadow * mShadow;
		Lighting * mLighting;
		FXAA * mFXAA;

		eEvKeyType mCurrentKey;
		EvKeyFrame mKeyFrames[eEvKeyType::MAX];
		EvGlobalParam mGlobalParam;

		eEvKeyType mTargetKey;
		int mFadeMode; // 0: none, 1: fade in, 2: fade out
		float mFadeTime;
		float mFadeMaxTime;
		
		EvKeyFrame mKeyFrameCurrent;
		EvKeyFrame mKeyFrameLast;
		EvParam mParam;
	};

}
