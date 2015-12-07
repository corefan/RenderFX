#include "MCloud.h"
#include "MRenderFX.h"

namespace Rad {

	Cloud::Cloud()
	{
		mTex_Layer1 = HWBufferManager::Instance()->LoadTexture("RenderFX/CloudLayer1.dds");
		mTex_Layer2 = HWBufferManager::Instance()->LoadTexture("RenderFX/CloudLayer2.dds");
		mTex_Layer3 = HWBufferManager::Instance()->LoadTexture("RenderFX/CloudLayer3.dds");
		mTex_Layer4 = HWBufferManager::Instance()->LoadTexture("RenderFX/CloudLayer4.dds");

		mTech_Cloud = ShaderFXManager::Instance()->Load("RenderFX.Cloud", "RenderFX/Cloud.mfx");
		mTech_Lighting = ShaderFXManager::Instance()->Load("CloudLighting", "RenderFX/CloudLighting.mfx");
		mTech_Blur = ShaderFXManager::Instance()->Load("RenderFX.Blur13", "RenderFX/Blur13.mfx");
		mTech_Shading = ShaderFXManager::Instance()->Load("CloudShading", "RenderFX/CloudShading.mfx");

		_initGeometry();
		_initRenderTarget();
	}

	Cloud::~Cloud()
	{
	}

	void Cloud::RenderLighting()
	{
		profile_code();

		RENDER_EVENT_ENTRY("CloudLighting");

		_cloud();
		_lighting();
		_blur();
	}

	void Cloud::Render(bool lighting)
	{
		_shading(lighting);
	}

	void Cloud::Resize(int w, int h)
	{
		_initRenderTarget();
	}

	void Cloud::_initGeometry()
	{
		int xSegments = 80;
		int zSegments = 80;

		int iVertexCount = (xSegments + 1) * (zSegments + 1);
		int iIndexCount = xSegments * zSegments * 2 * 3;
		int iPrimCount = xSegments * zSegments * 2;
		int iStride = 20;

		VertexDeclaration decl;
		decl.AddElement(eVertexSemantic::POSITION, eVertexType::FLOAT3);
		decl.AddElement(eVertexSemantic::TEXCOORD0, eVertexType::FLOAT2);

		VertexBufferPtr vb = HWBufferManager::Instance()->NewVertexBuffer(20, iVertexCount);

		float * vert = (float *)vb->Lock(eLockFlag::WRITE);

		float xstep = 2.0f / xSegments;
		float zstep = 2.0f / zSegments;
		float ustep = 1.0f / xSegments;
		float vstep = 1.0f / zSegments;

		for (int z = 0; z < zSegments + 1; ++z)
		{
			for (int x = 0; x < xSegments + 1; ++x)
			{
				float dx = (x - xSegments / 2.0f) / (float)xSegments;
				float dz = (z - zSegments / 2.0f) / (float)zSegments;
				float dt = Math::Sqrt(dx * dx + dz * dz) / (SQRT_2 * 0.5f);

				float px = -1 + x * xstep;
				float pz = +1 - z * zstep;
				float py = Math::Cos(dt * PI / 2);

				float u = 0 + x * ustep;
				float v = 0 + z * vstep;

				*vert++ = px; *vert++ = py; *vert++ = pz;

				*vert++ = u; *vert++ = v;
			}
		}

		vb->Unlock();

		IndexBufferPtr ib = HWBufferManager::Instance()->NewIndexBuffer(iIndexCount);
		short * idx = (short *)ib->Lock(eLockFlag::WRITE);
		{
			short row = 0, row_n = 0;
			short i, j;

			for (i = 0; i < zSegments; ++i)
			{
				row_n = row + xSegments + 1;

				for (j = 0; j < xSegments; ++j)
				{
					*idx++ = row + j;
					*idx++ = row + j + 1;
					*idx++ = row_n + j;

					*idx++ = row_n + j;
					*idx++ = row + j + 1;
					*idx++ = row_n + j + 1;

				}

				row += xSegments + 1;
			}
		}
		ib->Unlock();

		mRenderOp.vertexDeclarations[0] = decl;
		mRenderOp.vertexBuffers[0] = vb;
		mRenderOp.indexBuffer = ib;
		mRenderOp.primCount = iPrimCount;
		mRenderOp.primType = ePrimType::TRIANGLE_LIST;
	}

	void Cloud::_initGeometry2()
	{
		int iRings = 15, iSegments = 30;
		int iVertexCount = (iRings + 1) * (iSegments + 1);
		int iIndexCount = iRings * iSegments * 6;
		int iPrimCount = iIndexCount / 3;

		mRenderOp.vertexDeclarations[0].AddElement(eVertexSemantic::POSITION, eVertexType::FLOAT3);
		mRenderOp.vertexDeclarations[0].AddElement(eVertexSemantic::TEXCOORD0, eVertexType::FLOAT2);

		VertexBufferPtr vb = HWBufferManager::Instance()->NewVertexBuffer(sizeof(float) * 5, iVertexCount);

		float * vert = (float *)vb->Lock(eLockFlag::WRITE);
		{
			float fTileRingAngle = (PI_HALF / iRings);
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
					*vert++ = j / (float)iSegments;
					*vert++ = i / (float)iRings;
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

	void Cloud::_initRenderTarget()
	{
		int width = RenderFX::Instance()->GetColorBuffer()->GetWidth() / 2;
		int height = RenderFX::Instance()->GetColorBuffer()->GetHeight() / 2;

		mRT_Cloud = HWBufferManager::Instance()->NewRenderTarget(width, height, ePixelFormat::R16G16F);
		mRT_Lighting = HWBufferManager::Instance()->NewRenderTarget(width, height, ePixelFormat::R16F);
		mRT_Lighting1 = HWBufferManager::Instance()->NewRenderTarget(width, height, ePixelFormat::R16F);
	}

	void Cloud::_cloud()
	{
		RenderSystem::Instance()->SetRenderTarget(0, mRT_Cloud.c_ptr());
		RenderSystem::Instance()->PrepareRendering();

		RenderSystem::Instance()->SetViewport(0, 0, mRT_Cloud->GetWidth(), mRT_Cloud->GetHeight());

		RenderSystem::Instance()->Clear(eClearMode::COLOR, Float4(0, 0, 0), 1, 0);

		Camera * cam = World::Instance()->MainCamera();
		RenderSystem::Instance()->SetViewTM(cam->GetViewTM());
		RenderSystem::Instance()->SetProjTM(cam->GetProjTM());

		FX_Uniform * uTranslate = mTech_Cloud->GetPass(0)->GetUniform("gTranslate");
		FX_Uniform * uScale = mTech_Cloud->GetPass(0)->GetUniform("gScale");
		FX_Uniform * uMatWVP = mTech_Cloud->GetPass(0)->GetUniform("matWVP");
		FX_Uniform * uUVScale = mTech_Cloud->GetPass(0)->GetUniform("gUVScale");
		FX_Uniform * uUVScroll = mTech_Cloud->GetPass(0)->GetUniform("gUVScroll");

		FX_Uniform * uMass = mTech_Cloud->GetPass(0)->GetUniform("gMass");
		FX_Uniform * uWeight = mTech_Cloud->GetPass(0)->GetUniform("gWeight");

		float farclip = cam->GetFarClip() * 0.9f;
		Float3 pos = cam->GetPosition();

		float time = Root::Instance()->GetTime();
		float height = RenderFX::Instance()->GetEvParam()->CloudParam.Height;
		float curved = RenderFX::Instance()->GetEvParam()->CloudParam.Curved;

		uTranslate->SetConst(pos.x, pos.y + farclip * height, pos.z, 1);
		uScale->SetConst(farclip, farclip * curved, farclip, 1);

		RenderSystem::Instance()->Render(mTech_Cloud, &mRenderOp);
	}

	void Cloud::_lighting()
	{
		RenderSystem::Instance()->SetRenderTarget(0, mRT_Lighting.c_ptr());
		RenderSystem::Instance()->PrepareRendering();

		RenderSystem::Instance()->SetViewport(0, 0, mRT_Lighting->GetWidth(), mRT_Lighting->GetHeight());

		Camera * cam = World::Instance()->MainCamera();
		float farclip = cam->GetFarClip() * 0.9f;
		Float3 pos = cam->GetPosition();

		FX_Uniform * uMass = mTech_Lighting->GetPass(0)->GetUniform("gMass");
		FX_Uniform * uWeight = mTech_Lighting->GetPass(0)->GetUniform("gWeight");
		FX_Uniform * uUVScale = mTech_Lighting->GetPass(0)->GetUniform("gUVScale");
		FX_Uniform * uUVScroll = mTech_Lighting->GetPass(0)->GetUniform("gUVScroll");
		FX_Uniform * uUVSun = mTech_Lighting->GetPass(0)->GetUniform("gSunUV");
		FX_Uniform * uLightingParam = mTech_Lighting->GetPass(0)->GetUniform("gLightingParam");

		float time = Root::Instance()->GetTime();
		float height = RenderFX::Instance()->GetEvParam()->CloudParam.Height;
		float curved = RenderFX::Instance()->GetEvParam()->CloudParam.Curved;
		float mass = RenderFX::Instance()->GetEvParam()->CloudParam.Mass;
		Float4 wieghts = RenderFX::Instance()->GetEvParam()->CloudParam.Weights;
		Float4 uvScales = RenderFX::Instance()->GetEvParam()->CloudParam.UVScales;
		Float4 uvScrolls = RenderFX::Instance()->GetEvParam()->CloudParam.UVScrolls * time;
		float alpha = RenderFX::Instance()->GetEvParam()->CloudParam.Alpha;
		float alphaStrength = RenderFX::Instance()->GetEvParam()->CloudParam.AlphaStrength;
		float lightStrength = RenderFX::Instance()->GetEvParam()->CloudParam.LightStrength;
		float fadeStart = RenderFX::Instance()->GetEvParam()->CloudParam.FadeStart;
		float fadeStrength = RenderFX::Instance()->GetEvParam()->CloudParam.FadeStrength;
		Float3 ambient = RenderFX::Instance()->GetEvParam()->CloudParam.Ambient;
		Float3 diffuse = RenderFX::Instance()->GetEvParam()->CloudParam.Diffuse;
		float ambientScale = RenderFX::Instance()->GetEvParam()->CloudParam.AmbientScale;
		float diffuseScale = RenderFX::Instance()->GetEvParam()->CloudParam.DiffuseScale;
		Float2 windDir = Float2(1, 0);

		uMass->SetConst(mass, 0, 0, 0);
		uWeight->SetConst(wieghts);
		uUVScale->SetConst(uvScales);
		uUVScroll->SetConst(
			windDir.x * uvScrolls.x, windDir.y * uvScrolls.x, 
			windDir.x * uvScrolls.y, windDir.y * uvScrolls.y);
		uUVSun->SetConst(0.5f, 0.5f, 0.001f, 1);
		uLightingParam->SetConst(lightStrength, 0, 0, 0);

		RenderSystem::Instance()->SetTexture(0, mRT_Cloud->GetTexture().c_ptr());

		RenderSystem::Instance()->SetTexture(1, mTex_Layer1.c_ptr());
		RenderSystem::Instance()->SetTexture(2, mTex_Layer2.c_ptr());
		RenderSystem::Instance()->SetTexture(3, mTex_Layer3.c_ptr());
		RenderSystem::Instance()->SetTexture(4, mTex_Layer4.c_ptr());

		RenderSystem::Instance()->RenderScreenQuad(mTech_Lighting);
	}

	void Cloud::_blur()
	{
		for (int i = 0; i < 4; ++i)
		{
			RenderSystem::Instance()->SetRenderTarget(0, mRT_Lighting1.c_ptr());
			RenderSystem::Instance()->PrepareRendering();

			Float4 factorys[13];
			float inv_w = mRT_Lighting1->GetTexture()->_getInvWidth();
			float inv_h = mRT_Lighting1->GetTexture()->_getInvHeight();

			for (int i = 0; i < 13; ++i)
			{
				factorys[i].x = (i - 6) * inv_w;
				factorys[i].y = 0;
			}

			mTech_Blur->GetPass(0)->SetConst("gUVOffsets", factorys, 13);

			RenderSystem::Instance()->SetTexture(0, mRT_Lighting->GetTexture().c_ptr());

			RenderSystem::Instance()->RenderScreenQuad(mTech_Blur);

			//
			RenderSystem::Instance()->SetRenderTarget(0, mRT_Lighting.c_ptr());
			RenderSystem::Instance()->PrepareRendering();

			for (int i = 0; i < 13; ++i)
			{
				factorys[i].x = 0;
				factorys[i].y = (i - 6) * inv_h;
			}

			mTech_Blur->GetPass(0)->SetConst("gUVOffsets", factorys, 13);

			RenderSystem::Instance()->SetTexture(0, mRT_Lighting1->GetTexture().c_ptr());

			RenderSystem::Instance()->RenderScreenQuad(mTech_Blur);
		}
	}

	void Cloud::_shading(bool lighting)
	{
		Camera * cam = World::Instance()->MainCamera();
		Float3 pos = cam->GetPosition();
		float farclip = cam->GetFarClip() * 0.9f;

		FX_Uniform * uTranslate = mTech_Shading->GetPass(0)->GetUniform("gTranslate");
		FX_Uniform * uScale = mTech_Shading->GetPass(0)->GetUniform("gScale");
		FX_Uniform * uMatWVP = mTech_Shading->GetPass(0)->GetUniform("matWVP");
		FX_Uniform * uUVScale = mTech_Shading->GetPass(0)->GetUniform("gUVScale");
		FX_Uniform * uUVScroll = mTech_Shading->GetPass(0)->GetUniform("gUVScroll");

		FX_Uniform * uMass = mTech_Shading->GetPass(0)->GetUniform("gMass");
		FX_Uniform * uWeight = mTech_Shading->GetPass(0)->GetUniform("gWeight");
		FX_Uniform * uAlphaParam = mTech_Shading->GetPass(0)->GetUniform("gAlphaParam");
		FX_Uniform * uFadeParam = mTech_Shading->GetPass(0)->GetUniform("gFadeParam");
		FX_Uniform * uAmbient = mTech_Shading->GetPass(0)->GetUniform("gAmbient");
		FX_Uniform * uDiffuse = mTech_Shading->GetPass(0)->GetUniform("gDiffuse");

		float time = Root::Instance()->GetTime();
		float height = RenderFX::Instance()->GetEvParam()->CloudParam.Height;
		float curved = RenderFX::Instance()->GetEvParam()->CloudParam.Curved;
		float mass = RenderFX::Instance()->GetEvParam()->CloudParam.Mass;
		Float4 wieghts = RenderFX::Instance()->GetEvParam()->CloudParam.Weights;
		Float4 uvScales = RenderFX::Instance()->GetEvParam()->CloudParam.UVScales;
		Float4 uvScrolls = RenderFX::Instance()->GetEvParam()->CloudParam.UVScrolls * time;
		float alpha = RenderFX::Instance()->GetEvParam()->CloudParam.Alpha;
		float alphaStrength = RenderFX::Instance()->GetEvParam()->CloudParam.AlphaStrength;
		float lightStrength = RenderFX::Instance()->GetEvParam()->CloudParam.LightStrength;
		float fadeStart = RenderFX::Instance()->GetEvParam()->CloudParam.FadeStart;
		float fadeStrength = RenderFX::Instance()->GetEvParam()->CloudParam.FadeStrength;
		Float3 ambient = RenderFX::Instance()->GetEvParam()->CloudParam.Ambient;
		Float3 diffuse = RenderFX::Instance()->GetEvParam()->CloudParam.Diffuse;
		float ambientScale = RenderFX::Instance()->GetEvParam()->CloudParam.AmbientScale;
		float diffuseScale = RenderFX::Instance()->GetEvParam()->CloudParam.DiffuseScale;
		Float2 windDir = Float2(1, 0);

		ambient *= ambientScale;
		diffuse *= diffuseScale;

		//uTranslate->SetConst(pos.x, pos.y + farclip * 0.1f, pos.z, 1);
		uTranslate->SetConst(pos.x, pos.y + farclip * height, pos.z, 1);
		uScale->SetConst(farclip, farclip * curved, farclip, 1);
		uMass->SetConst(mass, 0, 0, 0);
		uWeight->SetConst(wieghts);
		uUVScale->SetConst(uvScales);
		uUVScroll->SetConst(
			windDir.x * uvScrolls.x, windDir.y * uvScrolls.x,
			windDir.x * uvScrolls.y, windDir.y * uvScrolls.y);

		uAlphaParam->SetConst(alpha, alphaStrength, 0, 0);
		uFadeParam->SetConst(fadeStart * farclip, 1 / (farclip - fadeStart * farclip), 1, 0);
		uAmbient->SetConst(ambient.r, ambient.g, ambient.b, 1);
		uDiffuse->SetConst(diffuse.r, diffuse.g, diffuse.b, 1);

		if (lighting)
			RenderSystem::Instance()->SetTexture(0, mRT_Lighting->GetTexture().c_ptr());
		else
			RenderSystem::Instance()->SetTexture(0, RenderHelper::Instance()->GetWhiteTexture().c_ptr());

		RenderSystem::Instance()->SetTexture(1, mTex_Layer1.c_ptr());
		RenderSystem::Instance()->SetTexture(2, mTex_Layer2.c_ptr());
		RenderSystem::Instance()->SetTexture(3, mTex_Layer3.c_ptr());
		RenderSystem::Instance()->SetTexture(4, mTex_Layer4.c_ptr());

		RenderSystem::Instance()->Render(mTech_Shading, &mRenderOp);
	}

}