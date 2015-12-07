#include "MSky.h"
#include "MRenderFX.h"
#include "MShaderFXManager.h"
#include "MHWBufferManager.h"
#include "MImage.h"

namespace Rad {

	Sky::Sky()
	{
		mTech = ShaderFXManager::Instance()->Load("Sky", "RenderFX/MSky.mfx");

		d_assert (mTech != NULL);

		int iRings = 30, iSegments = 30;
		int iVertexCount = (iRings + 1) * (iSegments + 1);
		int iIndexCount = iRings * iSegments * 6;
		int iPrimCount = iIndexCount / 3;

		mRenderOp.vertexDeclarations[0].AddElement(eVertexSemantic::POSITION, eVertexType::FLOAT3);

		VertexBufferPtr vb = HWBufferManager::Instance()->NewVertexBuffer(sizeof(float) * 3, iVertexCount);

		float * vert = (float *)vb->Lock(eLockFlag::WRITE);
		{
			float fTileRingAngle = (PI / iRings);
			float fTileSegAngle = (PI * 2 / iSegments);
			float r;
			short i, j;
			float x, y, z;

			for (i = 0; i <= iRings; ++i)
			{
				r = Math::Sin(i * fTileRingAngle);
				y = Math::Cos(i * fTileRingAngle);

				for (j = 0; j <= iSegments; ++j)
				{
					x = r * Math::Cos(j * fTileSegAngle);
					z = r * Math::Sin(j * fTileSegAngle);

					*vert++ = x;
					*vert++ = y;
					*vert++ = z;
				}

			}
		}
		vb->Unlock();

		IndexBufferPtr ib = HWBufferManager::Instance()->NewIndexBuffer(iIndexCount);
		short * idx = (short *)ib->Lock(eLockFlag::WRITE);
		{
			short row = 0, row_n = 0;
			short i, j;

			for (i = 0; i < iRings; ++i)
			{
				row_n = row + iSegments + 1;

				for (j = 0; j < iSegments; ++j)
				{
					*idx++ = row + j;
					*idx++ = row + j + 1;
					*idx++ = row_n + j;

					*idx++ = row_n + j;
					*idx++ = row + j + 1;
					*idx++ = row_n + j + 1;

				}

				row += iSegments + 1;
			}
		}
		ib->Unlock();

		mRenderOp.vertexBuffers[0] = vb;
		mRenderOp.indexBuffer = ib;
		mRenderOp.primCount = iPrimCount;
		mRenderOp.primType = ePrimType::TRIANGLE_LIST;
	}

	Sky::~Sky()
	{
	}

	void Sky::Render()
	{
		float skyLum = RenderFX::Instance()->GetEvParam()->SkyLum;
		float skyU = RenderFX::Instance()->GetEvParam()->SkyU;
		float skyV = RenderFX::Instance()->GetEvGlobalParam()->SkyVOff;
		const String & skyTexture = RenderFX::Instance()->GetEvGlobalParam()->SkyTexture;

		if (mTexture == NULL || mTexture->GetName() != skyTexture)
			mTexture = HWBufferManager::Instance()->LoadTexture(skyTexture, -1);

		RenderSystem * render = RenderSystem::Instance();
		Camera * cam = World::Instance()->MainCamera();
		Camera * currentCam = World::Instance()->GetCurrentRenderContext()->GetCamera();

		d_assert (cam != NULL);

		float farclip = cam->GetFarClip() * 0.9f;
		Float3 pos = cam->GetPosition();

		Mat4 matWVP;
		matWVP.MakeTransform(pos, Quat::Identity, Float3(farclip, farclip, farclip));
		matWVP *= RenderRegister::Instance()->GetViewTM();
		matWVP *= currentCam->GetProjTM();
		matWVP *= RenderSystem::Instance()->_getAdjustProjTM();

		bool hr = false;
		hr = mTech->GetPass(0)->SetConst("u_WVP", matWVP);
		hr = mTech->GetPass(0)->SetConst("u_Param", Float4(skyU, skyV, 1 / (1 + skyV), skyLum));
		hr = mTech->GetPass(0)->SetConst("u_InvTexHeight", Float4(mTexture->_getInvHeight(), 0, 0, 0));

		RenderSystem::Instance()->SetTexture(0, mTexture.c_ptr());
		render->Render(mTech, &mRenderOp);
	}


	//
	Sky2::Sky2()
	{
		mTech = ShaderFXManager::Instance()->Load("Sky2", "RenderFX/MSky2.mfx");
		d_assert (mTech != NULL && mTech->GetPassCount() > 0);

		_init();
	}

	Sky2::~Sky2()
	{
	}

	void Sky2::_init()
	{
		int iRings = 50, iSegments = 50;
		int iVertexCount = (iRings + 1) * (iSegments + 1);
		int iIndexCount = iRings * iSegments * 6;
		int iPrimCount = iIndexCount / 3;

		VertexDeclaration decl;
		decl.AddElement(eVertexSemantic::POSITION, eVertexType::FLOAT3);

		VertexBufferPtr vb = HWBufferManager::Instance()->NewVertexBuffer(12, iVertexCount);
		float * vert = (float *)vb->Lock(eLockFlag::WRITE);
		{
			float fTileRingAngle = (PI / iRings);
			float fTileSegAngle = (PI2 / iSegments);
			float r;
			short i, j;
			float x, y, z;

			for (i = 0; i <= iRings; ++i)
			{
				r = Math::Sin(i * fTileRingAngle);
				y = Math::Cos(i * fTileRingAngle);

				for (j = 0; j <= iSegments; ++j)
				{
					x = r * Math::Cos(j * fTileSegAngle);
					z = r * Math::Sin(j * fTileSegAngle);

					*vert++ = x;
					*vert++ = y;
					*vert++ = z;
				}

			}
		}
		vb->Unlock();

		IndexBufferPtr ib = HWBufferManager::Instance()->NewIndexBuffer(iIndexCount);
		short * idx = (short *)ib->Lock(eLockFlag::WRITE);
		{
			short row = 0, row_n = 0;
			short i, j;

			for (i = 0; i < iRings; ++i)
			{
				row_n = row + iSegments + 1;

				for (j = 0; j < iSegments; ++j)
				{
					*idx++ = row + j;
					*idx++ = row + j + 1;
					*idx++ = row_n + j;

					*idx++ = row_n + j;
					*idx++ = row + j + 1;
					*idx++ = row_n + j + 1;

				}

				row += iSegments + 1;
			}
		}
		ib->Unlock();

		mRenderOp.vertexDeclarations[0] = decl;
		mRenderOp.vertexBuffers[0] = vb;
		mRenderOp.indexBuffer = ib;
		mRenderOp.primCount = iPrimCount;
		mRenderOp.primType = ePrimType::TRIANGLE_LIST;
	}

	void Sky2::Render()
	{
		_update();

		RenderContext * context = World::Instance()->GetCurrentRenderContext();

		float farclip = World::Instance()->MainCamera()->GetFarClip() * 0.9f;
		Float3 pos = World::Instance()->MainCamera()->GetPosition();
		Mat4 form;

		form.MakeTransform(pos, Quat::Identity, Float3(farclip, farclip, farclip));

		RenderSystem::Instance()->SetWorldTM(form);

		RenderSystem::Instance()->Render(mTech, &mRenderOp);
	}

	void Sky2::_update()
	{
		Float3 sunDir = RenderFX::Instance()->GetEvParam()->SunDir;

		FX_Uniform * uSunDirVS = mTech->GetPass(0)->GetUniform("uSunDirVS");
		FX_Uniform * uSunDir = mTech->GetPass(0)->GetUniform("uSunDir");

		FX_Uniform * uInnerRadius = mTech->GetPass(0)->GetUniform("uInnerRadius");
		FX_Uniform * uCameraPos = mTech->GetPass(0)->GetUniform("uCameraPos");
		FX_Uniform * uScale = mTech->GetPass(0)->GetUniform("uScale");
		FX_Uniform * uScaleDepth = mTech->GetPass(0)->GetUniform("uScaleDepth");
		FX_Uniform * uScaleOverScaleDepth = mTech->GetPass(0)->GetUniform("uScaleOverScaleDepth");

		FX_Uniform * uKrESun = mTech->GetPass(0)->GetUniform("uKrESun");
		FX_Uniform * uKmESun = mTech->GetPass(0)->GetUniform("uKmESun");
		FX_Uniform * uKr4PI = mTech->GetPass(0)->GetUniform("uKr4PI");
		FX_Uniform * uKm4PI = mTech->GetPass(0)->GetUniform("uKm4PI");

		FX_Uniform * uInvWaveLength = mTech->GetPass(0)->GetUniform("uInvWaveLength");

		FX_Uniform * uG = mTech->GetPass(0)->GetUniform("uG");
		FX_Uniform * uG2 = mTech->GetPass(0)->GetUniform("uG2");

		FX_Uniform * uExposure = mTech->GetPass(0)->GetUniform("uExposure");

		uSunDirVS->SetConst(-sunDir.x, -sunDir.y, -sunDir.z, 0);
		uSunDir->SetConst(-sunDir.x, -sunDir.y, -sunDir.z, 0);

		{
			float Scale = 1.0f / (mOptions.OuterRadius - mOptions.InnerRadius);
			float ScaleDepth = (mOptions.OuterRadius - mOptions.InnerRadius) / 2.0f;
			float ScaleOverScaleDepth = Scale / ScaleDepth;

			uInnerRadius->SetConst(mOptions.InnerRadius, 0, 0, 0);
			uCameraPos->SetConst(0, mOptions.InnerRadius + (mOptions.OuterRadius-mOptions.InnerRadius)*mOptions.HeightPosition, 0, 0);

			uScale->SetConst(Scale, 0, 0, 0);
			uScaleDepth->SetConst(ScaleDepth, 0, 0, 0);
			uScaleOverScaleDepth->SetConst(ScaleOverScaleDepth, 0, 0, 0);
		}

		{
			float KrESun = mOptions.RayleighMultiplier * mOptions.SunIntensity;
			float KmESun = mOptions.MieMultiplier * mOptions.SunIntensity * 0;
			float Kr4PI  = mOptions.RayleighMultiplier * 4.0f * PI;
			float Km4PI = mOptions.MieMultiplier * 4.0f * PI;

			uKrESun->SetConst(KrESun, 0, 0, 0);
			uKmESun->SetConst(KmESun, 0, 0, 0);
			uKr4PI->SetConst(Kr4PI, 0, 0, 0);
			uKm4PI->SetConst(Km4PI, 0, 0, 0);
		}

		{
			Float3 invWaveLength = Float3(
				1 / Math::Pow(mOptions.WaveLength.x, 4.0f),
				1 / Math::Pow(mOptions.WaveLength.y, 4.0f),
				1 / Math::Pow(mOptions.WaveLength.z, 4.0f));

			uInvWaveLength->SetConst(invWaveLength.x, invWaveLength.y, invWaveLength.z, 0);
		}

		{
			uG->SetConst(mOptions.G, 0, 0, 0);
			uG2->SetConst(mOptions.G*mOptions.G, 0, 0, 0);
		}

		{
			//uExposure->SetConst(mOptions.Exposure, 0, 0, 0);
		}
	}

}