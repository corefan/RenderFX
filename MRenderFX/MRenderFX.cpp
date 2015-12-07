#include "MRenderFX.h"
#include "MResourceManager.h"
#include "MTerrainMesh.h"

namespace Rad {

	class RFX_ENTRY ShaderProviderGBuffer : public ShaderProvider
	{
	public:
		ShaderProviderGBuffer()
		{
			mBaseFX = ShaderFXManager::Instance()->Load("RenderFX.Base", "RenderFX/MBase.mfx");
			mBaseFXSkined = ShaderFXManager::Instance()->Load("RenderFX.BaseSkined", "RenderFX/MBase.mfx", "#define D_SKINED\n");
			mTerrainFX = ShaderFXManager::Instance()->Load("RenderFX.Terrain", "RenderFX/MTerrain.mfx");
		}

		virtual ~ShaderProviderGBuffer() {}

		virtual void ApplyShaderFX(RenderObject * able, int flags)
		{
			able->SetCurrentShaderFX(NULL);

			if (TYPE_OF(TerrainMesh, able->_getNode()))
				able->SetCurrentShaderFX(mTerrainFX);
			else if (!able->IsSkined())
				able->SetCurrentShaderFX(mBaseFX);
			else
				able->SetCurrentShaderFX(mBaseFXSkined);
		}

		virtual void ApplyLightingFX(Light * light, RenderObject * able) {}

	protected:
		ShaderFX * mBaseFX;
		ShaderFX * mBaseFXSkined;
		ShaderFX * mTerrainFX;
	};

	//
	class RenderPipelineGBuffer : public RenderPipeline
	{
		virtual ~RenderPipelineGBuffer() {}
		
		virtual void DoRender()
		{
			mRenderQueue.Clear();

			RenderContext * pRenderContext = World::Instance()->GetCurrentRenderContext();
			Camera * pCamera = pRenderContext->GetCamera();
			VisibleCullerPtr pVisibleCuller = pRenderContext->GetVisibleCuller();
			ShaderProviderPtr pShaderProvider = pRenderContext->GetShaderProvider();

			if (pVisibleCuller == NULL || pShaderProvider == NULL || pCamera == NULL)
				return ;

			Array<Node*> & visibleArray = pRenderContext->GetVisibleCuller()->GetNodeArray();
			for (int i = 0; i < visibleArray.Size(); ++i)
			{
				Node * n = visibleArray[i];
				if (n->GetRenderContextId() == pRenderContext->GetId())
				{
					visibleArray[i]->AddRenderQueue(&mRenderQueue);
				}
			}

			RenderSystem::Instance()->SetViewTM(pCamera->GetViewTM());
			RenderSystem::Instance()->SetProjTM(pCamera->GetProjTM());

			RenderSystem::Instance()->SetClipPlane(pCamera->GetNearClip(), pCamera->GetFarClip());
			RenderSystem::Instance()->SetTime(Root::Instance()->GetTime());

			RenderSystem::Instance()->SetLight(World::Instance()->MainLight());

			RenderSystem::Instance()->RenderScreenQuad(RenderFX::Instance()->GetClearFX());			

			Array<RenderObject *> & arr = mRenderQueue.GetSolidObjects();

			for (int i = 0; i < arr.Size(); ++i)
			{
				pShaderProvider->ApplyShaderFX(arr[i]);
			}

			if (arr.Size() > 0)
			{
				SolidSorter sorter;
				Sort(&arr[0], arr.Size(), sorter);
			}

			RenderFX::Instance()->E_RenderGBufferBegin();

			for (int i = 0; i < arr.Size(); ++i)
			{
				RenderSystem::Instance()->Render(arr[i]->GetCurrentShaderFX(), arr[i]);
			}

			RenderFX::Instance()->E_RenderGBufferEnd();
		}
	};

	//
	class RFX_ENTRY ShaderProviderShading : public ShaderProvider
	{
	public:
		ShaderProviderShading()
		{
			mBaseFX = ShaderFXManager::Instance()->Load("RenderFX.BaseShading", "RenderFX/MBaseShading.mfx");
			mBaseFXSkined = ShaderFXManager::Instance()->Load("RenderFX.BaseShadingSkined", "RenderFX/MBaseShading.mfx", "#define D_SKINED\n");
			mTerrainFX[0] = ShaderFXManager::Instance()->Load("RenderFX.TerrainShading1", "RenderFX/MTerrainShading.mfx", "#define D_LAYER1");
			mTerrainFX[1] = ShaderFXManager::Instance()->Load("RenderFX.TerrainShading2", "RenderFX/MTerrainShading.mfx", "#define D_LAYER2");
			mTerrainFX[2] = ShaderFXManager::Instance()->Load("RenderFX.TerrainShading3", "RenderFX/MTerrainShading.mfx", "#define D_LAYER3");
			mTerrainFX[3] = ShaderFXManager::Instance()->Load("RenderFX.TerrainShading4", "RenderFX/MTerrainShading.mfx", "#define D_LAYER4");
		}

		virtual ~ShaderProviderShading() {}

		virtual void ApplyShaderFX(RenderObject * able, int flags)
		{
			able->SetCurrentShaderFX(NULL);

			if (TYPE_OF(TerrainMesh, able->_getNode()))
			{
				TerrainMesh * tm = (TerrainMesh *)able->_getNode();
				able->SetCurrentShaderFX(mTerrainFX[tm->GetMaxLayer()]);
			}
			else if (able->GetShaderFX())
			{
				able->SetCurrentShaderFX(able->GetShaderFX());
			}
			else if (!able->IsSkined())
			{
				able->SetCurrentShaderFX(mBaseFX);
			}
			else
			{
				able->SetCurrentShaderFX(mBaseFXSkined);
			}
		}

		virtual void ApplyLightingFX(Light * light, RenderObject * able)
		{
			able->SetCurrentShaderFX(NULL);

			eLightType type = light->GetType();
			if (!TYPE_OF(TerrainMesh, able->_getNode()) && able->GetLightShaderFX(type) != NULL)
			{
				able->SetCurrentShaderFX(able->GetLightShaderFX(type));
			}
		}

	protected:
		ShaderFX * mBaseFX;
		ShaderFX * mBaseFXSkined;
		ShaderFX * mTerrainFX[Terrain::kMaxBlendLayers];
	};

	//
	ImplementSingleton(RenderFX);

	RenderFX::RenderFX()
	{
		Viewport vp = World::Instance()->MainRenderContext()->GetViewport();

		RenderTargetPtr pRenderTarget0 = HWBufferManager::Instance()->NewRenderTarget(vp.w, vp.h, ePixelFormat::R8G8B8A8);
		RenderTargetPtr pRenderTarget1 = HWBufferManager::Instance()->NewRenderTarget(vp.w, vp.h, ePixelFormat::R32F);
		DepthBufferPtr pDepthBuffer = HWBufferManager::Instance()->NewDepthBuffer(vp.w, vp.h, ePixelFormat::D24S8);

		mContext = World::Instance()->NewRenderContext(RFX_CONTETX_ORDER, "RenderFX");
		mContext->SetCamera(World::Instance()->MainCamera());
		mContext->SetRenderTarget(0, pRenderTarget0);
		mContext->SetRenderTarget(1, pRenderTarget1);
		mContext->SetDepthBuffer(pDepthBuffer);
		mContext->SetVisibleCuller(World::Instance()->MainRenderContext()->GetVisibleCuller());
		mContext->SetShaderProvider(new ShaderProviderGBuffer);
		mContext->SetRenderPipeline(new RenderPipelineGBuffer);
		mContext->SetColorClear(eClearMode::NONE, Float4(0, 0, 0));
		mContext->SetEnable(true);

		mColorBuffer = HWBufferManager::Instance()->NewRenderTarget(vp.w, vp.h, ePixelFormat::R16G16B16A16F);
		mColorBufferEx = HWBufferManager::Instance()->NewRenderTarget(vp.w, vp.h, ePixelFormat::R16G16B16A16F);
		World::Instance()->MainRenderContext()->SetRenderTarget(0, mColorBuffer);
		World::Instance()->MainRenderContext()->SetDepthBuffer(pDepthBuffer);
		//World::Instance()->MainRenderContext()->SetColorClear(eClearMode::NONE, Float4(0, 0, 0));
		World::Instance()->MainRenderContext()->SetShaderProvider(new ShaderProviderShading);

		mClearFX = ShaderFXManager::Instance()->Load("RenderFX.Clear", "RenderFX/Clear.mfx");

		mSky = new Sky;
		mSun = new Sun;
		mRain = new Rain;
		mSnow = new Snow;
		mStarfield = new Starfield;
		mCloud = new Cloud;
		mOcean = new Ocean;

		mHDR = new HDR;
		mGodRay = new GodRay;
		mSSAO = new SSAO;
		mShadow = new Shadow;
		mLighting = new Lighting;
		mFXAA = new FXAA;

		mKeyFrames[eEvKeyType::NIGHT].Time = 0;
		mKeyFrames[eEvKeyType::MORNING].Time = 7;
		mKeyFrames[eEvKeyType::NOON].Time = 12;
		mKeyFrames[eEvKeyType::EVENING].Time = 17;
		SetKeyType(eEvKeyType::MORNING);

		mFadeMode = 0;
		mFadeTime = 0;
		mFadeMaxTime = 0;

		World::Instance()->E_RenderSolidBegin += new cListener0<RenderFX>(this, &RenderFX::OnRenderSolid);
		World::Instance()->E_RenderAlphaEnd += new cListener0<RenderFX>(this, &RenderFX::OnRenderTransparent);
		World::Instance()->E_RenderContextEnd += new cListener0<RenderFX>(this, &RenderFX::OnRenderConextEnd);
		World::Instance()->E_RenderEnd += new cListener0<RenderFX>(this, &RenderFX::OnRenderEnd);

		World::Instance()->E_Load += new cListener0<RenderFX>(this, &RenderFX::OnSceneLoad);
		World::Instance()->E_Unload += new cListener0<RenderFX>(this, &RenderFX::OnSceneUnload);
	}

	RenderFX::~RenderFX()
	{
		safe_delete (mSky);
		safe_delete (mSun);
		safe_delete (mSnow);
		safe_delete (mRain);
		safe_delete (mStarfield);
		safe_delete (mCloud);
		safe_delete (mOcean);

		safe_delete (mHDR);
		safe_delete (mGodRay);
		safe_delete (mSSAO);
		safe_delete (mShadow);
		safe_delete (mLighting);
		safe_delete (mFXAA);

		World::Instance()->DeleteRenderContext(mContext);
	}

	float RenderFX::GetKeyTime(eEvKeyType type)
	{
		switch (type)
		{
		case eEvKeyType::NIGHT:
			return 0;

		case eEvKeyType::MORNING:
			return 7;

		case eEvKeyType::NOON:
			return 12;

		case eEvKeyType::EVENING:
			return 17;
		}

		d_assert (0);

		return 0;
	}

	void RenderFX::SetKeyType(eEvKeyType type)
	{
		mCurrentKey = type;
		mKeyFrameCurrent = mKeyFrames[type];
	}

	void RenderFX::FadeIn(eEvKeyType type, float time)
	{
		mTargetKey = type;
		mKeyFrameLast = mKeyFrameCurrent;
		mFadeMaxTime = time;
		mFadeMode = 1;
	}

	void RenderFX::FadeOut(float time)
	{
		mTargetKey = eEvKeyType::UNKNOWN;
		mKeyFrameLast = mKeyFrameCurrent;
		mFadeMode = 2;
	}

	float RenderFX::_getSkyU(float time)
	{
		const float mu[] = {0, 0.25f, 0.5f, 0.75f, 1};

		if (time < 7)
			return 0.25f * time / (7 - 0);
		else if (time < 17)
			return 0.25f + 0.5f * (time - 7) / (17 - 7);
		else if (time < 24)
			return 0.75f + 0.25f * (time - 17) / (24 - 17);

		d_assert (0);

		return 0;
	}

	float RenderFX::_getSunRoll(float time)
	{
		float k = time / 24;

		float degree = -90 + k * 360;

		if (degree < 0)
			degree += 360;

		return degree;
	}

	float RenderFX::_getMoonRoll(float time)
	{
		float k = 1 - time / 24;

		float degree = 90 + k * 360;

		if (degree >= 360)
			degree -= 360;

		return degree;
	}

	float RenderFX::_getLightRoll(float time)
	{
		float roll = 0;

		if (time >= 6 && time <= 18)
			roll = _getSunRoll(time);
		else
			roll = _getMoonRoll(time);

		roll = Max(roll, 30.0f);
		roll = Min(roll, 150.f);

		return roll;
	}

	void RenderFX::Update(float frameTime)
	{
		if (mFadeMode == 0)
		{
			mKeyFrameCurrent = mKeyFrames[mCurrentKey];
		}
		else if (mFadeMode == 1) // in
		{
			d_assert (mTargetKey < eEvKeyType::MAX);

			mFadeTime += frameTime;
			if (mFadeTime > mFadeMaxTime)
				mFadeTime = mFadeMaxTime;

			EvKeyFrame::Lerp(
				mKeyFrameCurrent, 
				mKeyFrameLast, 
				mKeyFrames[mTargetKey], 
				mFadeTime / mFadeMaxTime);
		}
		else if (mFadeMode == 2) // out
		{
			mFadeTime -= frameTime;
			if (mFadeTime < 0)
				mFadeTime = 0;

			EvKeyFrame::Lerp(
				mKeyFrameCurrent, 
				mKeyFrameLast, 
				mKeyFrames[mCurrentKey], 
				mFadeTime / mFadeMaxTime);

			if (mFadeTime <= 0)
			{
				mFadeMode = 0;
			}
		}

		EvParam & p = mParam;
		const EvKeyFrame & kf = mKeyFrameCurrent;

		Camera * cam = World::Instance()->MainCamera();
		float farclip = cam->GetFarClip() * 0.9f;

		float sunRoll = _getSunRoll(kf.Time);
		float moonRoll = _getMoonRoll(kf.Time);
		float lightRoll = _getLightRoll(kf.Time);
		float lightYaw = 0;
		float lightPicth = mGlobalParam.SunPitch;

		if (lightPicth >= 0)
			lightPicth = Math::Clamp(lightPicth, 1.0f, 89.0f);
		else
			lightPicth = Math::Clamp(lightPicth, -89.0f, -1.0f);

		p.StarfieldLum = kf.StarfieldLum;
		p.SkyLum = kf.SkyLum;
		p.SkyU = _getSkyU(kf.Time);

		p.SunDir = _makeSunDir(lightYaw, lightPicth, sunRoll);
		p.SunPos = cam->GetPosition() - farclip * p.SunDir;
		p.SunColor = kf.SunColor;
		p.SunLum = kf.SunLum;
		p.SunSize = kf.SunSize;
		p.SunPower = kf.SunPower;

		p.MoonDir = _makeMoonDir(lightYaw, lightPicth, moonRoll);
		p.MoonPos = cam->GetPosition() - farclip * p.MoonDir;
		p.MoonLum = kf.MoonLum;
		p.MoonSize = kf.MoonSize;
		p.MoonPhase = kf.MoonPhase;

		p.WindDir = kf.WindDir;

		p.LightDir = _makeLightDir(p.SunDir, p.MoonDir);
		p.LightAmbient = kf.LightAmbient;
		p.LightDiffuse = kf.LightDiffuse;
		p.LightSpecular = kf.LightSpecular;

		p.FogStart = kf.FogStart;
		p.FogEnd = kf.FogEnd;
		p.FogColor = kf.FogColor;

		p.HdrParam = kf.HdrParam;
		p.GodRayParam = kf.GodRayParam;
		p.GrassParam = kf.GrassParam;
		p.CloudParam = kf.CloudParam;
		p.OceanParam = kf.OceanParam;

		World::Instance()->MainLight()->SetDirection(mParam.LightDir);
		World::Instance()->MainLight()->SetAmbient(mParam.LightAmbient);
		World::Instance()->MainLight()->SetDiffuse(mParam.LightDiffuse);
		World::Instance()->MainLight()->SetSpecular(mParam.LightSpecular);

		RenderSystem::Instance()->SetFog(mParam.FogColor, mParam.FogStart, mParam.FogEnd);

		GrassManager::Instance()->SetWaveParam(
			Float2(mParam.WindDir.x, mParam.WindDir.z),
			mParam.GrassParam.WaveSpeed,
			mParam.GrassParam.WaveStrength);

		GrassManager::Instance()->SetVisibleRadius(mParam.GrassParam.VisibleRadius);

		if (mSnow)
			mSnow->Update(frameTime);

		if (mRain)
			mRain->Update(frameTime);

		if (mOcean)
			mOcean->Update(frameTime);
	}

	void RenderFX::Resize(int w, int h)
	{
		mContext->GetRenderTarget(0)->Resize(w, h);
		mContext->GetRenderTarget(1)->Resize(w, h);
		mContext->GetDepthBuffer()->Resize(w, h);
		mContext->SetViewport(Viewport(0, 0, w, h));

		if (mColorBuffer != NULL)
			mColorBuffer->Resize(w, h);

		if (mColorBufferEx != NULL)
			mColorBufferEx->Resize(w, h);

		if (mCloud)
			mCloud->Resize(w, h);

		if (mOcean)
			mOcean->Resize(w, h);

		if (mHDR)
			mHDR->Resize(w, h);

		if (mGodRay)
			mGodRay->Resize(w, h);

		if (mSSAO)
			mSSAO->Resize(w, h);

		if (mShadow)
			mShadow->Resize(w, h);

		if (mLighting)
			mLighting->Resize(w, h);
		
		E_Resize(w, h);
	}

#define RFX_MAGIC MAKE_DWORD('R', 'F', 'X', '0')

	void RenderFX::Load(const String & filename)
	{
		DataStreamPtr stream = ResourceManager::Instance()->OpenResource(filename);
		if (stream == NULL)
		{
			d_log("!: RenderFX load failed, '%s'.", filename.c_str());
			return ;
		}

		ISerializerD IS(stream);

		int magic;
		IS >> magic;

		d_assert (magic == RFX_MAGIC);

		int ckId = 0;
		while (IS.Read(&ckId, sizeof(int)) && ckId != 0)
		{
			if (ckId == 0x0001)
			{
				int kId;
				IS >> kId;

				d_assert (kId < eEvKeyType::MAX);

				mKeyFrames[kId].Serialize(IS);
			}
			else
			{
				d_log("?: unknown render fx file.");
				break;
			}
		}
	}

	void RenderFX::Save(const String & filename)
	{
		FILE * fp = fopen(filename.c_str(), "wb");
		if (!fp)
		{
			d_log("!: RenderFX save failed, '%s'.", filename.c_str());
			return ;
		}

		OSerializerF OS(fp, true);

		OS << (int)(RFX_MAGIC);

		for (int i = 0; i < eEvKeyType::MAX; ++i)
		{
			OS << (int)0x0001;
			OS << i;

			mKeyFrames[i].Serialize(OS);
		}
	}

	Float3 RenderFX::_makeSunDir(float y, float p, float r)
	{
		Mat4 matY, matP, matR;

		y = Math::DegreeToRadian(y);
		p = Math::DegreeToRadian(p);
		r = Math::DegreeToRadian(r);

		matY.MakeRotationY(y);
		matP.MakeRotationX(p);
		matR.MakeRotationZ(r);

		matR *= matY;
		matR *= matP;

		Float3 dir = Float3(-1, 0, 0);
		dir.TransformN(matR);

		return dir;
	}

	Float3 RenderFX::_makeMoonDir(float y, float p, float r)
	{
		Mat4 matY, matP, matR;

		y = Math::DegreeToRadian(y);
		p = Math::DegreeToRadian(p);
		r = Math::DegreeToRadian(r);

		matY.MakeRotationY(y);
		matP.MakeRotationX(p);
		matR.MakeRotationZ(r);

		matR *= matY;
		matR *= matP;

		Float3 dir = Float3(-1, 0, 0);
		dir.TransformN(matR);

		return dir;
	}

	Float3 RenderFX::_makeLightDir(const Float3 & sunDir, const Float3 & moonDir)
	{
		Float3 dir;

		if (sunDir.y <= 0 )
			dir = sunDir;
		else
			dir = moonDir;

		d_assert(fabs(dir.y) < 1.0f && "not support!");

		Float3 xzDir = Float3(-dir.x, 0, -dir.z);

		xzDir.Normalize();

		if (xzDir.Dot(-dir) > Math::Cos(PI / 6))
		{
			Float3 right = Float3::Cross(Float3(0, 1, 0), dir);

			right.Normalize();

			Quat q;

			q.FromAxis(right, PI / 6);

			xzDir.TransformQ(q);

			dir = -xzDir;
		}

		return dir;
	}

	void RenderFX::OnSceneLoad()
	{
		const String & filename = World::Instance()->GetFilename();

		String fxfile = FileHelper::ReplaceExternName(filename, "rfx");

		Load(fxfile);
	}

	void RenderFX::OnSceneUnload()
	{
	}

	void RenderFX::_updateColorTexture()
	{
		RenderSystem::Instance()->SetRenderTarget(0, NULL);
		RenderSystem::Instance()->PrepareRendering();

		RenderSystem::Instance()->StretchRect(mColorBufferEx.c_ptr(), NULL, mColorBuffer.c_ptr(), NULL);
	}

	void RenderFX::OnRenderSolid()
	{
		RenderContext * pRenderContext = World::Instance()->GetCurrentRenderContext();
		RenderTargetPtr pNormalRT = mContext->GetRenderTarget(0);
		RenderTargetPtr pDepthRT = mContext->GetRenderTarget(1);

		if (mSky)
			mSky->Render();

		if (mStarfield)
			mStarfield->Render();

		if (mSun)
			mSun->Render();

		if (mCloud)
			mCloud->Render();
	}

	void RenderFX::OnRenderTransparent()
	{
		if (mSnow)
			mSnow->Render();

		if (mRain)
			mRain->Render();
	}

	void RenderFX::OnRenderConextEnd()
	{
		RenderContext * pRenderContext = World::Instance()->GetCurrentRenderContext();
		RenderTargetPtr pNormalRT = mContext->GetRenderTarget(0);
		RenderTargetPtr pDepthRT = mContext->GetRenderTarget(1);

		if (pRenderContext == mContext)
		{
			RenderSystem::Instance()->SetRenderTarget(0, NULL);
			RenderSystem::Instance()->SetRenderTarget(1, NULL);
			RenderSystem::Instance()->SetRenderTarget(2, NULL);
			RenderSystem::Instance()->SetRenderTarget(3, NULL);
			RenderSystem::Instance()->SetDepthBuffer(NULL);

			if (mCloud != NULL)
				mCloud->RenderLighting();

			if (mShadow != NULL)
				mShadow->Render(pDepthRT->GetTexture().c_ptr());

			if (mLighting != NULL)
				mLighting->Render(
					pNormalRT->GetTexture().c_ptr(),
					pDepthRT->GetTexture().c_ptr(),
					mShadow != NULL ? mShadow->GetShadowTexture() : RenderHelper::Instance()->GetWhiteTexture().c_ptr());

			RenderSystem::Instance()->SetSpecialTexture(RFX_SPECIAL_SAMPLER_LIGHTING,
				mLighting ? mLighting->GetTexture() : RenderHelper::Instance()->GetWhiteTexture().c_ptr());
		}

		if (mOcean && pRenderContext == World::Instance()->MainRenderContext())
		{
			_updateColorTexture();

			RenderSystem::Instance()->SetRenderTarget(0, pRenderContext->GetRenderTarget(0).c_ptr());
			RenderSystem::Instance()->PrepareRendering();

			mOcean->Render(pDepthRT->GetTexture().c_ptr(), mColorBufferEx->GetTexture().c_ptr());
		}
	}

	void RenderFX::OnRenderEnd()
	{
		RenderTargetPtr pRenderTarget = World::Instance()->MainRenderContext()->GetRenderTarget(0);
		if (pRenderTarget != NULL)
		{
			RenderTargetPtr pNormalRT = mContext->GetRenderTarget(0);
			RenderTargetPtr pDepthRT = mContext->GetRenderTarget(1);

			RenderSystem::Instance()->SetCamera(World::Instance()->MainCamera());
			RenderSystem::Instance()->SetWorldTM(Mat4::Identity);

			int currentRT = 0;

			if (mSSAO != NULL)
			{
				mSSAO->Render(mColorBuffer.c_ptr(), pDepthRT->GetTexture().c_ptr(), pNormalRT->GetTexture().c_ptr());
			}

			if (mGodRay != NULL)
			{
				mGodRay->Render(mColorBuffer.c_ptr(), pDepthRT->GetTexture().c_ptr());
			}

			if (mHDR != NULL)
			{
				mHDR->Render(mColorBufferEx.c_ptr(), mColorBuffer->GetTexture().c_ptr(), pDepthRT->GetTexture().c_ptr());
				currentRT = 1;
			}

			if (mFXAA != NULL)
			{
				if (currentRT == 0)
					_updateColorTexture();

				mFXAA->Render(NULL, mColorBufferEx->GetTexture().c_ptr());
			}
			else
			{
				RenderHelper::Instance()->DrawSumit(
					World::Instance()->MainRenderContext()->GetViewport(),
					pRenderTarget->GetTexture().c_ptr());
			}
		}
	}
}