#pragma once

#include "MRenderFXEntry.h"
#include "MRenderSystem.h"

namespace Rad {

	class RFX_ENTRY Cloud
	{
	public:
		Cloud();
		~Cloud();

		void 
			Resize(int w, int h);
		void
			RenderLighting();
		void 
			Render(bool lighting = true);

		Texture *
			GetCloudTexture() { return mRT_Cloud->GetTexture().c_ptr(); }
		Texture *
			GetLightingTexture() { return mRT_Lighting->GetTexture().c_ptr(); }

	protected:
		void 
			_initGeometry();
		void 
			_initGeometry2();
		void 
			_initRenderTarget();

		void 
			_cloud();
		void
			_lighting();
		void 
			_blur();
		void 
			_shading(bool lighting);

	protected:
		bool mDomeable;
		RenderOp mRenderOp;

		ShaderFX * mTech_Cloud;
		ShaderFX * mTech_Lighting;
		ShaderFX * mTech_Blur;
		ShaderFX * mTech_Shading;

		RenderTargetPtr mRT_Cloud;
		RenderTargetPtr mRT_Lighting;
		RenderTargetPtr mRT_Lighting1;
		TexturePtr mTex_Layer1;
		TexturePtr mTex_Layer2;
		TexturePtr mTex_Layer3;
		TexturePtr mTex_Layer4;
	};

}