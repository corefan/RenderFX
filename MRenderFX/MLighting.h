/*
*	Lighting
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "MRenderFXEntry.h"

namespace Rad {

#define N_LIGHTS 4

	class RFX_ENTRY Lighting
	{
	public:
		Lighting();
		~Lighting();

		void
			Resize(int w, int h);
		void
			Render(Texture * normalTex, Texture * depthTex, Texture * shadowTex);

		Texture *
			GetTexture() { return mRenderTarget->GetTexture().c_ptr(); }

	protected:
		void
			_render(const FixedArray<Light *, N_LIGHTS> & lights);

	protected:
		RenderTargetPtr mRenderTarget;

		ShaderFX * mDirLightFX;
		ShaderFX * mPointLightFX[N_LIGHTS];
		ShaderFX * mSpotLightFX[N_LIGHTS];
	};
}