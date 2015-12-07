#include "MGodRay.h"
#include "MRenderFX.h"

namespace Rad {

	GodRay::GodRay()
	{
		_initGeometry();
		_initTechnique();
		_initRenderTarget();
	}

	GodRay::~GodRay()
	{
	}

	void GodRay::_initGeometry()
	{
		int iVertexCount = 4;
		int iPrimCount = 2;

		mRenderOp.vertexDeclarations[0].AddElement(eVertexSemantic::POSITION, eVertexType::FLOAT3);
		mRenderOp.vertexDeclarations[0].AddElement(eVertexSemantic::TEXCOORD0, eVertexType::FLOAT2);

		VertexBufferPtr vb = HWBufferManager::Instance()->NewVertexBuffer(20, iVertexCount);

		float * vert = (float *)vb->Lock(eLockFlag::WRITE);
		{
			float x = 0, y = 0, z = 0;

			*vert++ = x; *vert++ = y; *vert++ = z;
			*vert++ = 0; *vert++ = 0;

			*vert++ = x; *vert++ = y; *vert++ = z;
			*vert++ = 1; *vert++ = 0;

			*vert++ = x; *vert++ = y; *vert++ = z;
			*vert++ = 0; *vert++ = 1;

			*vert++ = x; *vert++ = y; *vert++ = z;
			*vert++ = 1; *vert++ = 1;
		}
		vb->Unlock();

		mRenderOp.vertexBuffers[0] = vb;

		mRenderOp.primCount = iPrimCount;
		mRenderOp.primType = ePrimType::TRIANGLE_STRIP;
	}

	void GodRay::_initTechnique()
	{
		mTech_SunOcc = ShaderFXManager::Instance()->Load("RenderFX.GodRay_SunOcc", "RenderFX/GodRay_SunOcc.mfx");
		mTech_Sun = ShaderFXManager::Instance()->Load("RenderFX.GodRay_Sun", "RenderFX/GodRay_Sun.mfx");
		mTech_GodRay = ShaderFXManager::Instance()->Load("RenderFX.GodRay", "RenderFX/GodRay.mfx");
		mTech_Blur = ShaderFXManager::Instance()->Load("RenderFX.Blur13", "RenderFX/Blur13.mfx");
		mTech_Blend = ShaderFXManager::Instance()->Load("RenderFX.GodRay_Blend", "RenderFX/GodRay_Blend.mfx");

		d_assert (mTech_Sun && mTech_GodRay && mTech_Blur && mTech_Blend);
	}

	void GodRay::_initRenderTarget()
	{
		int w = RenderFX::Instance()->GetColorBuffer()->GetWidth();
		int h = RenderFX::Instance()->GetColorBuffer()->GetHeight();

		mRenderTarget = HWBufferManager::Instance()->NewRenderTarget(w / 2, h / 2, ePixelFormat::R16F);
		mRenderTarget1 = HWBufferManager::Instance()->NewRenderTarget(w / 2, h / 2, ePixelFormat::R16F);
	}

	void GodRay::Resize(int w, int h)
	{
		_initRenderTarget();
	}

	void GodRay::Render(RenderTarget * rt, Texture * depthTex)
	{
		RENDER_EVENT_ENTRY("GodRay");

		_occlusion();
		_renderSun();
		_godray(depthTex);
		_blur();
		_blend(rt);
	}

	void GodRay::_occlusion()
	{
		mOccLighting = 1;
		mOccRadius = 1;

		/*
		Camera * cam = World::Instance()->MainCamera();

		float sunSize = RenderFX::Instance()->GetEvParam()->GodRayParam.SunSize;
		Float3 sunDir = RenderFX::Instance()->GetEvParam()->SunDir;
		float farclip = cam->GetFarClip() * 0.899f;
		Float3 pos = cam->GetPosition() - farclip * sunDir;

		mTech_SunOcc->GetPass(0)->SetConst("gTransform", pos.x, pos.y, pos.z, sunSize);

		int pixels = 0, pixelsToRendering = 0;
		{
			RenderSystem::Instance()->SetMaterialRenderState(
				eFillMode::SOLID, eCullMode::NONE, eDepthMode::NONE, eBlendMode::OPACITY);

			RenderSystem::Instance()->BeginQuery();

			RenderSystem::Instance()->Render(mTech_SunOcc, &mRenderOp);

			pixels = RenderSystem::Instance()->EndQuery();
		}

		{
			RenderSystem::Instance()->SetMaterialRenderState(
				eFillMode::SOLID, eCullMode::NONE, eDepthMode::N_LESS_EQUAL, eBlendMode::OPACITY);

			RenderSystem::Instance()->BeginQuery();

			RenderSystem::Instance()->Render(mTech_SunOcc, &mRenderOp);

			pixelsToRendering = RenderSystem::Instance()->EndQuery();
		}

		float k = float(pixelsToRendering + 1) / float(pixels + 1);

		if (k < 0.8f)
		{
			k = (0.8f - k) / 0.6f;

			mOccLighting = Math::Lerp(1.0f, 2.5f, k);
			mOccRadius = Math::Lerp(1.0f, 2.0f, k);
		}
		else
		{
			mOccLighting = 1;
			mOccRadius = 1;
		}
		*/
	}

	void GodRay::_renderSun()
	{
		RenderSystem::Instance()->SetRenderTarget(0, mRenderTarget.c_ptr());
		RenderSystem::Instance()->PrepareRendering();

		RenderSystem::Instance()->SetViewport(0, 0, mRenderTarget->GetWidth(), mRenderTarget->GetHeight());

		RenderSystem::Instance()->Clear(eClearMode::COLOR, Float4(0, 0, 0, 1), 1, 0);

		RenderSystem::Instance()->SetViewTM(World::Instance()->MainCamera()->GetViewTM());
		RenderSystem::Instance()->SetProjTM(World::Instance()->MainCamera()->GetProjTM());

		Float3 sunColor = RenderFX::Instance()->GetEvParam()->SunColor;
		float sunLum = RenderFX::Instance()->GetEvParam()->GodRayParam.SunLum;
		float sunInner = RenderFX::Instance()->GetEvParam()->GodRayParam.SunInner;
		float sunPower = RenderFX::Instance()->GetEvParam()->GodRayParam.SunPower;
		Float3 sunPos = RenderFX::Instance()->GetEvParam()->SunPos;
		Float3 sunDir = RenderFX::Instance()->GetEvParam()->SunDir;
		float sunSize = RenderFX::Instance()->GetEvParam()->GodRayParam.SunSize * mOccRadius;

		mTech_Sun->GetPass(0)->SetConst("gTransform", sunPos.x, sunPos.y, sunPos.z, sunSize);
		mTech_Sun->GetPass(0)->SetConst("gSunParam", sunInner, 1 / (0.5f - sunInner), 1, 1);

		RenderSystem::Instance()->Render(mTech_Sun, &mRenderOp);
	}

	void GodRay::_godray(Texture * depthTex)
	{
		RenderSystem::Instance()->SetRenderTarget(0, mRenderTarget1.c_ptr());
		RenderSystem::Instance()->PrepareRendering();

		RenderSystem::Instance()->SetViewport(0, 0, mRenderTarget1->GetWidth(), mRenderTarget1->GetHeight());

		Float3 sunDir = RenderFX::Instance()->GetEvParam()->SunDir;
		Float3 sunPos = RenderFX::Instance()->GetEvParam()->SunPos;
		float sunSize = RenderFX::Instance()->GetEvParam()->GodRayParam.SunSize;
		float uvStep = RenderFX::Instance()->GetEvParam()->GodRayParam.UVStep;

		const Mat4 & matVP = World::Instance()->MainCamera()->GetViewProjTM();

		Float3 sunUV = sunPos * matVP;
		sunUV.x = (sunUV.x + 1) / 2;
		sunUV.y = (-sunUV.y + 1) / 2;

		mTech_GodRay->GetPass(0)->SetConst("gGodRayParam", sunUV.x, sunUV.y, 1 / uvStep, 0);

		RenderSystem::Instance()->SetTexture(0, depthTex);
		RenderSystem::Instance()->SetTexture(1, mRenderTarget->GetTexture().c_ptr());

		RenderSystem::Instance()->RenderScreenQuad(mTech_GodRay);
	}

	void GodRay::_blur()
	{
		RenderSystem::Instance()->SetRenderTarget(0, mRenderTarget.c_ptr());
		RenderSystem::Instance()->PrepareRendering();

		RenderSystem::Instance()->SetViewport(0, 0, mRenderTarget->GetWidth(), mRenderTarget->GetHeight());

		Float4 factorys[13];
		float inv_w = mRenderTarget->GetTexture()->_getInvWidth();
		float inv_h = mRenderTarget->GetTexture()->_getInvHeight();

		for (int i = 0; i < 13; ++i)
		{
			factorys[i].x = (i - 6) * inv_w;
			factorys[i].y = 0;
		}

		mTech_Blur->GetPass(0)->SetConst("gUVOffsets", factorys, 13);

		RenderSystem::Instance()->SetTexture(0, mRenderTarget1->GetTexture().c_ptr());

		RenderSystem::Instance()->RenderScreenQuad(mTech_Blur);
		
		//
		RenderSystem::Instance()->SetRenderTarget(0, mRenderTarget1.c_ptr());
		RenderSystem::Instance()->PrepareRendering();

		for (int i = 0; i < 13; ++i)
		{
			factorys[i].x = 0;
			factorys[i].y = (i - 6) * inv_h;
		}

		mTech_Blur->GetPass(0)->SetConst("gUVOffsets", factorys, 13);

		RenderSystem::Instance()->SetTexture(0, mRenderTarget->GetTexture().c_ptr());

		RenderSystem::Instance()->RenderScreenQuad(mTech_Blur);
	}

	void GodRay::_blend(RenderTarget * rt)
	{
		RenderSystem::Instance()->SetRenderTarget(0, rt);
		RenderSystem::Instance()->PrepareRendering();

		if (rt != NULL)
			RenderSystem::Instance()->SetViewport(0, 0, rt->GetWidth(), rt->GetHeight());
		else
			RenderSystem::Instance()->SetViewport(World::Instance()->MainRenderContext()->GetViewport());

		float sunLum = RenderFX::Instance()->GetEvParam()->GodRayParam.SunLum;
		Float3 sunColor = RenderFX::Instance()->GetEvParam()->SunColor * sunLum * mOccLighting;
		float blendWeight = RenderFX::Instance()->GetEvParam()->GodRayParam.Weight;

		mTech_Blend->GetPass(0)->SetConst("gSunColor", sunColor.r, sunColor.g, sunColor.b, 1);
		mTech_Blend->GetPass(0)->SetConst("gBlendWeight", blendWeight, 0, 0, 0);

		RenderSystem::Instance()->SetTexture(0, mRenderTarget->GetTexture().c_ptr());

		RenderSystem::Instance()->RenderScreenQuad(mTech_Blend);
	}

}