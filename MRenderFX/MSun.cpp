#include "MSun.h"
#include "MRenderFX.h"
#include "MShaderFXManager.h"
#include "MHWBufferManager.h"

namespace Rad {

	Sun::Sun()
	{
		mTech = ShaderFXManager::Instance()->Load("Sun", "RenderFX/MSun.mfx");

		d_assert (mTech != NULL);

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

	Sun::~Sun()
	{
	}

	void Sun::Render()
	{
		Float3 sunColor = RenderFX::Instance()->GetEvParam()->SunColor;
		float sunLum = RenderFX::Instance()->GetEvParam()->SunLum;
		float sunPower = RenderFX::Instance()->GetEvParam()->SunPower;
		Float3 sunDir = RenderFX::Instance()->GetEvParam()->SunDir;
		float sunSize = RenderFX::Instance()->GetEvParam()->SunSize;
		Float3 sunPos = RenderFX::Instance()->GetEvParam()->SunPos;

		FX_Uniform * uTransform = mTech->GetPass(0)->GetUniform("gTransform");
		FX_Uniform * uSunColor = mTech->GetPass(0)->GetUniform("gSunColor");
		FX_Uniform * uSunParam = mTech->GetPass(0)->GetUniform("gSunParam");

		uTransform->SetConst(sunPos.x, sunPos.y, sunPos.z, sunSize);
		uSunColor->SetConst(sunColor.r, sunColor.g, sunColor.b, 1);
		uSunParam->SetConst(sunPower, sunLum, 0, 0);

		RenderSystem::Instance()->SetWorldTM(Mat4::Identity);
		RenderSystem::Instance()->Render(mTech, &mRenderOp);
	}

}