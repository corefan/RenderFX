#include "MHDR.h"
#include "MRenderFX.h"

namespace Rad {

	HDR::HDR()
	{
		mTech_DownScale2x = ShaderFXManager::Instance()->Load("RenderFX.DownScale2x", "RenderFX/DownScale2x.mfx");
		mTech_DownScale4x = ShaderFXManager::Instance()->Load("RenderFX.DownScale4x", "RenderFX/DownScale4x.mfx");
		mTech_LumInitial = ShaderFXManager::Instance()->Load("RenderFX.HDR_LumInitial", "RenderFX/HDR_LumInitial.mfx");
		mTech_LumFinal = ShaderFXManager::Instance()->Load("RenderFX.HDR_LumFinal", "RenderFX/HDR_LumFinal.mfx");
		mTech_Adapted = ShaderFXManager::Instance()->Load("RenderFX.HDR_Adapted", "RenderFX/HDR_Adapted.mfx");
		mTech_Bright = ShaderFXManager::Instance()->Load("RenderFX.HDR_Bright", "RenderFX/HDR_Bright.mfx");
		mTech_Blur = ShaderFXManager::Instance()->Load("RenderFX.Blur13", "RenderFX/Blur13.mfx");
		mTech_ToneMap = ShaderFXManager::Instance()->Load("RenderFX.HDR_ToneMap", "RenderFX/HDR_ToneMap.mfx");

		d_assert (mTech_DownScale2x != NULL && mTech_DownScale4x != NULL );
		d_assert (mTech_LumInitial != NULL  && mTech_LumFinal != NULL );
		d_assert (mTech_Adapted != NULL  && mTech_Bright != NULL && mTech_ToneMap != NULL );
		d_assert (mTech_Blur != NULL  && mTech_ToneMap != NULL );

		_initRT();
	}

	HDR::~HDR()
	{
		mRT_Quad = NULL;
		mRT_Bloom0 = NULL;
		mRT_Bloom0_1 = NULL;
		mRT_Bloom1 = NULL;
		mRT_Bloom1_1 = NULL;
		mRT_Bloom2 = NULL;
		mRT_Bloom2_1 = NULL;
		mRT_64x64 = NULL;
		mRT_16x16 = NULL;
		mRT_4x4 = NULL;
		mRT_1x1 = NULL;
		mRT_1x1_0 = NULL;
		mRT_1x1_1 = NULL;
	}

	void HDR::Render(RenderTarget * rt, Texture * sceneTex, Texture * depthTex)
	{
		RENDER_EVENT_ENTRY("HDR");

		RenderSystem::Instance()->SetRenderTarget(0, NULL);
		RenderSystem::Instance()->SetRenderTarget(1, NULL);
		RenderSystem::Instance()->SetRenderTarget(2, NULL);
		RenderSystem::Instance()->SetRenderTarget(3, NULL);
		RenderSystem::Instance()->SetDepthBuffer(NULL);

		_downScale(sceneTex);

		_lumInitial();
		_lumDownScale_16x();
		_lumDownScale_4x();
		_lumFinal();
		_lumAdapted();
		_bright();

		_final(rt, sceneTex, depthTex);
	}

	void HDR::Resize(int w, int h)
	{
		_initRT();
	}

	void HDR::_initRT()
	{
		int w = RenderFX::Instance()->GetColorBuffer()->GetWidth();
		int h = RenderFX::Instance()->GetColorBuffer()->GetHeight();

		mRT_Quad = HWBufferManager::Instance()->NewRenderTarget(w / 2, h / 2, ePixelFormat::R16G16B16A16F);
		mRT_Bloom0 = HWBufferManager::Instance()->NewRenderTarget(w / 2, h / 2, ePixelFormat::R16G16B16A16F);
		mRT_Bloom0_1 = HWBufferManager::Instance()->NewRenderTarget(w / 2, h / 2, ePixelFormat::R16G16B16A16F);
		mRT_Bloom1 = HWBufferManager::Instance()->NewRenderTarget(w / 4, h / 4, ePixelFormat::R16G16B16A16F);
		mRT_Bloom1_1 = HWBufferManager::Instance()->NewRenderTarget(w / 4, h / 4, ePixelFormat::R16G16B16A16F);
		mRT_Bloom2 = HWBufferManager::Instance()->NewRenderTarget(w / 8, h / 8, ePixelFormat::R16G16B16A16F);
		mRT_Bloom2_1 = HWBufferManager::Instance()->NewRenderTarget(w / 8, h / 8, ePixelFormat::R16G16B16A16F);
		mRT_64x64 = HWBufferManager::Instance()->NewRenderTarget(64, 64, ePixelFormat::R16F);
		mRT_16x16 = HWBufferManager::Instance()->NewRenderTarget(16, 16, ePixelFormat::R16F);
		mRT_4x4 = HWBufferManager::Instance()->NewRenderTarget(4, 4, ePixelFormat::R16F);
		mRT_1x1 = HWBufferManager::Instance()->NewRenderTarget(1, 1, ePixelFormat::R16F);
		mRT_1x1_0 = HWBufferManager::Instance()->NewRenderTarget(1, 1, ePixelFormat::R16F);
		mRT_1x1_1 = HWBufferManager::Instance()->NewRenderTarget(1, 1, ePixelFormat::R16F);
	}

	void HDR::_downScale(Texture * sceneTex)
	{
		RenderSystem::Instance()->SetRenderTarget(0, mRT_Quad.c_ptr());
		RenderSystem::Instance()->PrepareRendering();

		RenderSystem::Instance()->SetViewport(0, 0, mRT_Quad->GetWidth(), mRT_Quad->GetHeight());

		float invWidth = 1.0f / mRT_Quad->GetWidth();
		float invHeight = 1.0f / mRT_Quad->GetHeight();

		Float4 uvOffs[4] = {
			Float4(0, 0, 0, 0), Float4(invWidth, 0, 0, 0),
			Float4(0, invHeight, 0, 0), Float4(invWidth, invHeight, 0, 0),
		};

		mTech_DownScale2x->GetPass(0)->SetConst("gUVOffsets", uvOffs, 4);

		RenderSystem::Instance()->SetTexture(0, sceneTex);

		RenderSystem::Instance()->RenderScreenQuad(mTech_DownScale2x);
	}

	void HDR::_lumInitial()
	{
		RenderSystem::Instance()->SetRenderTarget(0, mRT_64x64.c_ptr());
		RenderSystem::Instance()->PrepareRendering();

		RenderSystem::Instance()->SetViewport(0, 0, mRT_64x64->GetWidth(), mRT_64x64->GetHeight());

		Float4 uvOffs[9] = {
			Float4(0, 0, 0, 0), Float4(1, 0, 0, 0), Float4(2, 0, 0, 0),
			Float4(0, 1, 0, 0), Float4(1, 1, 0, 0), Float4(2, 1, 0, 0),
			Float4(0, 2, 0, 0), Float4(1, 2, 0, 0), Float4(2, 2, 0, 0)
		};

		float invW = 1.0f / mRT_Quad->GetWidth();
		float invH = 1.0f / mRT_Quad->GetHeight();

		for (int i = 0; i < 9; ++i)
		{
			uvOffs[i] *= Float4(invW, invH, 0, 0);
		}

		mTech_LumInitial->GetPass(0)->SetConst("gUVOffsets", uvOffs, 9);

		RenderSystem::Instance()->SetTexture(0, mRT_Quad->GetTexture().c_ptr());

		RenderSystem::Instance()->RenderScreenQuad(mTech_LumInitial);
	}

	void HDR::_lumDownScale_16x()
	{
		RenderSystem::Instance()->SetRenderTarget(0, mRT_16x16.c_ptr());
		RenderSystem::Instance()->PrepareRendering();

		RenderSystem::Instance()->SetViewport(0, 0, mRT_16x16->GetWidth(), mRT_16x16->GetHeight());

		Float4 uvOffs[16] = {
			Float4(0, 0, 0, 0), Float4(1, 0, 0, 0), Float4(2, 0, 0, 0), Float4(3, 0, 0, 0),
			Float4(0, 1, 0, 0), Float4(1, 1, 0, 0), Float4(2, 1, 0, 0), Float4(3, 1, 0, 0),
			Float4(0, 2, 0, 0), Float4(1, 2, 0, 0), Float4(2, 2, 0, 0), Float4(3, 2, 0, 0),
			Float4(0, 3, 0, 0), Float4(1, 3, 0, 0), Float4(2, 3, 0, 0), Float4(3, 3, 0, 0)
		};

		for (int i = 0; i < 16; ++i)
		{
			uvOffs[i] *= 1 / 64.0f;
		}

		mTech_DownScale4x->GetPass(0)->SetConst("gUVOffsets", uvOffs, 16);

		RenderSystem::Instance()->SetTexture(0, mRT_64x64->GetTexture().c_ptr());

		RenderSystem::Instance()->RenderScreenQuad(mTech_DownScale4x);
	}

	void HDR::_lumDownScale_4x()
	{
		RenderSystem::Instance()->SetRenderTarget(0, mRT_4x4.c_ptr());
		RenderSystem::Instance()->PrepareRendering();

		RenderSystem::Instance()->SetViewport(0, 0, mRT_4x4->GetWidth(), mRT_4x4->GetHeight());

		float invWidth = 1.0f / mRT_Quad->GetWidth();

		Float4 uvOffs[4] = {
			Float4(0, 0, 0, 0), Float4(1, 0, 0, 0), 
			Float4(0, 1, 0, 0), Float4(1, 1, 0, 0)
		};

		for (int i = 0; i < 4; ++i)
		{
			uvOffs[i] *= 1 / 16.0f;
		}

		mTech_DownScale2x->GetPass(0)->SetConst("gUVOffsets", uvOffs, 4);

		RenderSystem::Instance()->SetTexture(0, mRT_16x16->GetTexture().c_ptr());

		RenderSystem::Instance()->RenderScreenQuad(mTech_DownScale4x);
	}

	void HDR::_lumFinal()
	{
		RenderSystem::Instance()->SetRenderTarget(0, mRT_1x1_1.c_ptr());
		RenderSystem::Instance()->PrepareRendering();

		RenderSystem::Instance()->SetViewport(0, 0, mRT_1x1->GetWidth(), mRT_1x1->GetHeight());

		Float4 uvOffs[16] = {
			Float4(0, 0, 0, 0), Float4(1, 0, 0, 0), Float4(2, 0, 0, 0), Float4(3, 0, 0, 0),
			Float4(0, 1, 0, 0), Float4(1, 1, 0, 0), Float4(2, 1, 0, 0), Float4(3, 1, 0, 0),
			Float4(0, 2, 0, 0), Float4(1, 2, 0, 0), Float4(2, 2, 0, 0), Float4(3, 2, 0, 0),
			Float4(0, 3, 0, 0), Float4(1, 3, 0, 0), Float4(2, 3, 0, 0), Float4(3, 3, 0, 0)
		};

		for (int i = 0; i < 16; ++i)
		{
			uvOffs[i] *= 1 / 4.0f;
		}

		mTech_LumFinal->GetPass(0)->SetConst("gUVOffsets", uvOffs, 16);

		RenderSystem::Instance()->SetTexture(0, mRT_4x4->GetTexture().c_ptr());

		RenderSystem::Instance()->RenderScreenQuad(mTech_LumFinal);
	}

	void HDR::_lumAdapted()
	{
		RenderSystem::Instance()->SetRenderTarget(0, mRT_1x1.c_ptr());
		RenderSystem::Instance()->PrepareRendering();

		RenderSystem::Instance()->SetViewport(0, 0, mRT_1x1->GetWidth(), mRT_1x1->GetHeight());

		Float4 k;
		k.y = 1 - pow(0.98f, Root::Instance()->GetFrameTime() * 30);
		k.x = 1 - k.y;

		mTech_Adapted->GetPass(0)->SetConst("gK", k);

		RenderSystem::Instance()->SetTexture(0, mRT_1x1_0->GetTexture().c_ptr());
		RenderSystem::Instance()->SetTexture(1, mRT_1x1_1->GetTexture().c_ptr());

		RenderSystem::Instance()->RenderScreenQuad(mTech_Adapted);

		// save current
		//RenderHelper::Instance()->DrawSumit(Viewport(0, 0, 1, 1), mRT_1x1->GetTexture().c_ptr(), mRT_1x1_0.c_ptr());
		RenderSystem::Instance()->StretchRect(mRT_1x1_0.c_ptr(), NULL, mRT_1x1.c_ptr(), NULL);
	}

	void HDR::_bright()
	{
		// bright
		{
			RenderSystem::Instance()->SetRenderTarget(0, mRT_Bloom0.c_ptr());
			RenderSystem::Instance()->PrepareRendering();

			RenderSystem::Instance()->SetViewport(0, 0, mRT_Bloom0->GetWidth(), mRT_Bloom0->GetHeight());

			float threshold = RenderFX::Instance()->GetEvParam()->HdrParam.Threshold;

			mTech_Bright->GetPass(0)->SetConst("gThreshold", Float4(threshold, 0, 0, 0));

			RenderSystem::Instance()->SetTexture(0, mRT_Quad->GetTexture().c_ptr());

			RenderSystem::Instance()->RenderScreenQuad(mTech_Bright);
		}

		_blurH(mRT_Bloom0_1.c_ptr(), mRT_Bloom0->GetTexture().c_ptr());
		_blurV(mRT_Bloom0.c_ptr(), mRT_Bloom0_1->GetTexture().c_ptr());

		// down scale 
		{
			RenderSystem::Instance()->SetRenderTarget(0, mRT_Bloom1.c_ptr());
			RenderSystem::Instance()->PrepareRendering();

			RenderSystem::Instance()->SetViewport(0, 0, mRT_Bloom1->GetWidth(), mRT_Bloom1->GetHeight());

			float invWidth = 1.0f / mRT_Bloom0->GetWidth();
			float invHeight = 1.0f / mRT_Bloom0->GetHeight();

			Float4 uvOffs[4] = {
				Float4(0, 0, 0, 0), Float4(1, 0, 0, 0), 
				Float4(0, 1, 0, 0), Float4(1, 1, 0, 0)
			};

			for (int i = 0; i < 4; ++i)
			{
				uvOffs[i].x *= invWidth;
				uvOffs[i].y *= invHeight;
			}

			mTech_DownScale2x->GetPass(0)->SetConst("gUVOffsets", uvOffs, 4);

			RenderSystem::Instance()->SetTexture(0, mRT_Bloom0->GetTexture().c_ptr());

			RenderSystem::Instance()->RenderScreenQuad(mTech_DownScale2x);
		}

		_blurH(mRT_Bloom1_1.c_ptr(), mRT_Bloom1->GetTexture().c_ptr());
		_blurV(mRT_Bloom1.c_ptr(), mRT_Bloom1_1->GetTexture().c_ptr());

		// down scale 
		{
			RenderSystem::Instance()->SetRenderTarget(0, mRT_Bloom2.c_ptr());
			RenderSystem::Instance()->PrepareRendering();

			RenderSystem::Instance()->SetViewport(0, 0, mRT_Bloom2->GetWidth(), mRT_Bloom2->GetHeight());

			float invWidth = 1.0f / mRT_Bloom1->GetWidth();
			float invHeight = 1.0f / mRT_Bloom1->GetHeight();

			Float4 uvOffs[4] = {
				Float4(0, 0, 0, 0), Float4(1, 0, 0, 0), 
				Float4(0, 1, 0, 0), Float4(1, 1, 0, 0)
			};

			for (int i = 0; i < 4; ++i)
			{
				uvOffs[i].x *= invWidth;
				uvOffs[i].y *= invHeight;
			}

			mTech_DownScale2x->GetPass(0)->SetConst("gUVOffsets", uvOffs, 4);

			RenderSystem::Instance()->SetTexture(0, mRT_Bloom1->GetTexture().c_ptr());

			RenderSystem::Instance()->RenderScreenQuad(mTech_DownScale2x);
		}

		_blurH(mRT_Bloom2_1.c_ptr(), mRT_Bloom2->GetTexture().c_ptr());
		_blurV(mRT_Bloom2.c_ptr(), mRT_Bloom2_1->GetTexture().c_ptr());
	}

	float hdr_gaussian(float x, float gm)
	{
		gm = 1.0f / gm;

		float e = -x * x * 0.5f * gm * gm;

		return  1.0f / sqrt(PI2) * gm * exp(e);
	}

	void HDR::_blurH(RenderTarget * rt, Texture * tex)
	{
		RenderSystem::Instance()->SetRenderTarget(0, rt);
		RenderSystem::Instance()->PrepareRendering();

		RenderSystem::Instance()->SetViewport(0, 0, rt->GetWidth(), rt->GetHeight());

		float inv_w = tex->_getInvWidth();

		Float4 factorys[13];
		for (int i = 0; i < 13; ++i)
		{
			factorys[i].x = (i - 6) * inv_w;
			factorys[i].y = 0;
		}

		mTech_Blur->GetPass(0)->SetConst("gUVOffsets", factorys, 13);

		RenderSystem::Instance()->SetTexture(0, tex);

		RenderSystem::Instance()->RenderScreenQuad(mTech_Blur);
	}

	void HDR::_blurV(RenderTarget * rt, Texture * tex)
	{
		RenderSystem::Instance()->SetRenderTarget(0, rt);
		RenderSystem::Instance()->PrepareRendering();

		RenderSystem::Instance()->SetViewport(0, 0, rt->GetWidth(), rt->GetHeight());

		float inv_h = tex->_getInvHeight();

		Float4 factorys[13];
		for (int i = 0; i < 13; ++i)
		{
			factorys[i].x = 0;
			factorys[i].y = (i - 6) * inv_h;
		}

		mTech_Blur->GetPass(0)->SetConst("gUVOffsets", factorys, 13);

		RenderSystem::Instance()->SetTexture(0, tex);

		RenderSystem::Instance()->RenderScreenQuad(mTech_Blur);
	}

	void HDR::_final(RenderTarget * rt, Texture * sceneTex, Texture * depthTex)
	{
		RenderSystem::Instance()->SetRenderTarget(0, rt);
		RenderSystem::Instance()->PrepareRendering();

		if (rt != NULL)
			RenderSystem::Instance()->SetViewport(0, 0, rt->GetWidth(), rt->GetHeight());
		else
			RenderSystem::Instance()->SetViewport(World::Instance()->MainRenderContext()->GetViewport());

		float exposure = RenderFX::Instance()->GetEvParam()->HdrParam.Exposure;
		float weight0 = RenderFX::Instance()->GetEvParam()->HdrParam.Weights.x;
		float weight1 = RenderFX::Instance()->GetEvParam()->HdrParam.Weights.y;
		float weight2 = RenderFX::Instance()->GetEvParam()->HdrParam.Weights.z;
		float sharpScale = RenderFX::Instance()->GetEvParam()->HdrParam.SharpScale;
		float sharpFadeStart = RenderFX::Instance()->GetEvParam()->HdrParam.SharpFadeStart;
		float sharpFadeEnd = RenderFX::Instance()->GetEvParam()->HdrParam.SharpFadeEnd;

		Float4 filter_detal[8] = {
			Float4(-1, -1, 0), Float4(0, -1, 0), Float4(1, -1, 0),
			Float4(-1,  0, 0),					 Float4(1,  0, 0),
			Float4(-1,  1, 0), Float4(0,  1, 0), Float4(1,  1, 0),
		};
		
		for (int i = 0; i < 8; ++i)
		{
			filter_detal[i].x *= sceneTex->_getInvWidth();
			filter_detal[i].y *= sceneTex->_getInvHeight();
		}

		FX_Uniform * uWeights = mTech_ToneMap->GetPass(0)->GetUniform("gBloomWeight");
		FX_Uniform * uExposure = mTech_ToneMap->GetPass(0)->GetUniform("gExposure");
		FX_Uniform * uSharpParam = mTech_ToneMap->GetPass(0)->GetUniform("gSharpParam");
		FX_Uniform * uUVOffsets = mTech_ToneMap->GetPass(0)->GetUniform("gUVOffsets");

		uWeights->SetConst(weight0, weight1, weight2, 0);
		uExposure->SetConst(exposure, 0, 0, 0);
		uSharpParam->SetConst(sharpScale, sharpFadeStart, 1 / (sharpFadeEnd - sharpFadeStart), 0);
		uUVOffsets->SetConst(filter_detal, 8);

		RenderSystem::Instance()->SetTexture(0, mRT_1x1->GetTexture().c_ptr());
		RenderSystem::Instance()->SetTexture(1, sceneTex);
		RenderSystem::Instance()->SetTexture(2, depthTex);
		RenderSystem::Instance()->SetTexture(3, mRT_Bloom0->GetTexture().c_ptr());
		RenderSystem::Instance()->SetTexture(4, mRT_Bloom1->GetTexture().c_ptr());
		RenderSystem::Instance()->SetTexture(5, mRT_Bloom2->GetTexture().c_ptr());

		RenderSystem::Instance()->RenderScreenQuad(mTech_ToneMap);
	}
}