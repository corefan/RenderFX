#include "MSSAO.h"
#include "MRenderFX.h"

namespace Rad {

	SSAO::SSAO()
	{
		mTech_Ao = ShaderFXManager::Instance()->Load("RenderFX.SSAO", "RenderFX/SSAO.mfx");
		mTech_Blur = ShaderFXManager::Instance()->Load("RenderFX.Blur13", "RenderFX/Blur13.mfx");
		mTech_Blend = ShaderFXManager::Instance()->Load("RenderFX.SSAO_Blend", "RenderFX/SSAO_Blend.mfx");

		mTex_Random = HWBufferManager::Instance()->LoadTexture("RenderFX/random.png");

		d_assert (mTech_Ao && mTech_Blend && mTech_Blur);

		_init();
	}

	SSAO::~SSAO()
	{
	}

	void SSAO::_init()
	{
		int width = RenderFX::Instance()->GetColorBuffer()->GetWidth();
		int height = RenderFX::Instance()->GetColorBuffer()->GetHeight();

		mRT_Quad0 = HWBufferManager::Instance()->NewRenderTarget(width / 2, height / 2, ePixelFormat::R16F);
		mRT_Quad1 = HWBufferManager::Instance()->NewRenderTarget(width / 2, height / 2, ePixelFormat::R16F);
	}

	void SSAO::Resize(int w, int h)
	{
		_init();
	}

	void SSAO::Render(RenderTarget * rt, Texture * depthTex, Texture * normalTex)
	{
		RENDER_EVENT_ENTRY("SSAO");

		_renderAo(depthTex, normalTex);

		_blur();

		_blend(rt);
	}

	void SSAO::_renderAo(Texture * depthTex, Texture * normalTex)
	{
		RenderSystem::Instance()->SetRenderTarget(0, mRT_Quad0.c_ptr());
		RenderSystem::Instance()->PrepareRendering();

		RenderSystem::Instance()->SetViewport(0, 0, mRT_Quad0->GetWidth(), mRT_Quad0->GetHeight());

		RenderSystem::Instance()->Clear(eClearMode::COLOR, Float4(1, 1, 1, 1), 1, 0);

		const Float3 * corners = World::Instance()->MainCamera()->GetViewCorner();
		Float3 cornerLeftTop = corners[4];
		Float3 cornerRightDir = corners[5] - corners[4];
		Float3 cornerDownDir = corners[6] - corners[4];

		FX_Uniform * uCornerLeftTop = mTech_Ao->GetPass(0)->GetUniform("gCornerLeftTop");
		FX_Uniform * uCornerRightDir = mTech_Ao->GetPass(0)->GetUniform("gCornerRightDir");
		FX_Uniform * uCornerDownDir = mTech_Ao->GetPass(0)->GetUniform("gCornerDownDir");
		FX_Uniform * uMatProj = mTech_Ao->GetPass(0)->GetUniform("ptMat");

		uCornerLeftTop->SetConst(cornerLeftTop.x, cornerLeftTop.y, cornerLeftTop.z, 0);
		uCornerRightDir->SetConst(cornerRightDir.x, cornerRightDir.y, cornerRightDir.z, 0);
		uCornerDownDir->SetConst(cornerDownDir.x, cornerDownDir.y, cornerDownDir.z, 0);
		uMatProj->SetConst(World::Instance()->MainCamera()->GetProjTM());

		RenderSystem::Instance()->SetTexture(0, depthTex);
		RenderSystem::Instance()->SetTexture(1, normalTex);
		RenderSystem::Instance()->SetTexture(2, mTex_Random.c_ptr());

		RenderSystem::Instance()->RenderScreenQuad(mTech_Ao);
	}

	void SSAO::_blur()
	{
		RenderSystem::Instance()->SetRenderTarget(0, mRT_Quad1.c_ptr());
		RenderSystem::Instance()->PrepareRendering();

		RenderSystem::Instance()->SetViewport(0, 0, mRT_Quad0->GetWidth(), mRT_Quad0->GetHeight());

		Float4 factorys[13];
		float inv_w = mRT_Quad0->GetTexture()->_getInvWidth();
		float inv_h = mRT_Quad0->GetTexture()->_getInvHeight();

		for (int i = 0; i < 13; ++i)
		{
			factorys[i].x = (i - 6) * inv_w;
			factorys[i].y = 0;
		}

		mTech_Blur->GetPass(0)->SetConst("gUVOffsets", factorys, 13);

		RenderSystem::Instance()->SetTexture(0, mRT_Quad0->GetTexture().c_ptr());

		RenderSystem::Instance()->RenderScreenQuad(mTech_Blur);

		//
		RenderSystem::Instance()->SetRenderTarget(0, mRT_Quad0.c_ptr());
		RenderSystem::Instance()->PrepareRendering();

		for (int i = 0; i < 13; ++i)
		{
			factorys[i].x = 0;
			factorys[i].y = (i - 6) * inv_h;
		}

		mTech_Blur->GetPass(0)->SetConst("gUVOffsets", factorys, 13);

		RenderSystem::Instance()->SetTexture(0, mRT_Quad1->GetTexture().c_ptr());

		RenderSystem::Instance()->RenderScreenQuad(mTech_Blur);
	}

	void SSAO::_blend(RenderTarget * rt)
	{
		RenderSystem::Instance()->SetRenderTarget(0, rt);
		RenderSystem::Instance()->PrepareRendering();

		if (rt != NULL)
			RenderSystem::Instance()->SetViewport(0, 0, rt->GetWidth(), rt->GetHeight());
		else
			RenderSystem::Instance()->SetViewport(World::Instance()->MainRenderContext()->GetViewport());

		RenderSystem::Instance()->SetTexture(0, mRT_Quad0->GetTexture().c_ptr());

		RenderSystem::Instance()->RenderScreenQuad(mTech_Blend);
	}
}