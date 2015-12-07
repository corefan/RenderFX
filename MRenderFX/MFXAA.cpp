#include "MFXAA.h"

namespace Rad {

	FXAA::FXAA()
	{
		mReduceMin = 1.0f / 128.0f;
		mReduceMul = 1.0f / 8.0f;
		mSpanMax = 8.0f;

		mShaderFX = ShaderFXManager::Instance()->Load("RenderFX.FXAA", "RenderFX/FXAA2.mfx");
		d_assert(mShaderFX != NULL && mShaderFX->GetPassCount() > 0);
	}

	FXAA::~FXAA()
	{
	}

	void FXAA::Render(RenderTarget * rt, Texture * tex)
	{
		RenderSystem::Instance()->SetRenderTarget(0, rt);
		RenderSystem::Instance()->PrepareRendering();

		if (rt)
			RenderSystem::Instance()->SetViewport(0, 0, rt->GetWidth(), rt->GetHeight());
		else
			RenderSystem::Instance()->SetViewport(World::Instance()->MainRenderContext()->GetViewport());

		float invW = tex->_getInvWidth();
		float invH = tex->_getInvHeight();

		mShaderFX->GetPass(0)->SetConst("gParam", mReduceMin, mReduceMul, mSpanMax, 0);
		mShaderFX->GetPass(0)->SetConst("inv_width_height", invW, invH, 0, 0);

		RenderSystem::Instance()->SetTexture(0, tex);

		RenderSystem::Instance()->RenderScreenQuad(mShaderFX);
	}

}
