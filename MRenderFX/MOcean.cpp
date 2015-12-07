#include "MOcean.h"
#include "MProjectionGrid.h"
#include "MRenderFX.h"
#include "MRoot.h"
#include "MWorld.h"

namespace Rad {

    Ocean::Ocean()
		: mCamera(NULL)
		, mRenderContext(NULL)
    {
		mPosition = Float3(0, 3 * UNIT_METRES, 0);
		mProjGrid = new ProjectedGrid();

		mTech = ShaderFXManager::Instance()->Load("RenderFX.Ocean.mfx", "RenderFX/Ocean.mfx");
		mTech_UnderWater = ShaderFXManager::Instance()->Load("RenderFX.UnderOcean.mfx", "RenderFX.UnderOcean.mfx");
		mTex_Wave = HWBufferManager::Instance()->LoadTexture("RenderFX/waves2.dds", -1);
		mTex_Fresnel = HWBufferManager::Instance()->LoadTexture("RenderFX/Fresnel.bmp", -1);
		mTex_Normal0 = HWBufferManager::Instance()->LoadTexture("RenderFX/WaterNormal1.tga", -1);
		mTex_Normal1 = HWBufferManager::Instance()->LoadTexture("RenderFX/WaterNormal2.tga", -1);

		const Viewport & vp = World::Instance()->MainRenderContext()->GetViewport();

		int w = vp.w;
		int h = vp.h;

		mCamera = new Camera;
		mRenderTarget = HWBufferManager::Instance()->NewRenderTarget(w, h);
		mDepthBuffer = HWBufferManager::Instance()->NewDepthBuffer(w, h, ePixelFormat::D24S8);

		mRenderPipeline = new RenderPipelineReflection;
		
		mRenderContext = World::Instance()->NewRenderContext(0, "OceanReflection");
		mRenderContext->SetCamera(mCamera);
		mRenderContext->SetViewport(Viewport(0, 0, w, h));
		mRenderContext->SetRenderTarget(0, mRenderTarget);
		mRenderContext->SetDepthBuffer(mDepthBuffer);
		mRenderContext->SetVisibleCuller(new VisibleCullerStandard);
		mRenderContext->SetShaderProvider(new ShaderProviderStandard);
		mRenderContext->SetRenderPipeline(mRenderPipeline);
    }

    Ocean::~Ocean()
    {
		if (mRenderContext != NULL)
			World::Instance()->DeleteRenderContext(mRenderContext);
		
		delete mCamera;
		delete mProjGrid;
    }

	void Ocean::Update(float frameTime)
	{
		mProjGrid->SetElevation(mPosition.y);
		mProjGrid->Update(frameTime);

		if (mRenderContext == NULL)
			return ;

		bool underWater = _isUnderWater();
		if (!underWater)
		{
			mRenderContext->SetEnable(true);

			Plane plane(0, 1, 0, -mPosition.y);

			Camera * mainCam = World::Instance()->MainCamera();
			mCamera->SetPosition(mainCam->GetWorldPosition());
			mCamera->SetRotation(mainCam->GetWorldRotation());
			mCamera->SetClipPlane(mainCam->GetNearClip(), mainCam->GetFarClip());
			mCamera->SetAspect(mainCam->GetAspect());
			mCamera->SetFov(mainCam->GetFov());
			mCamera->Reflect(plane);

			plane.normal.y = 1;
			mRenderPipeline->SetPlane(plane);
		}
		
		mRenderContext->SetEnable(!underWater);
	}

    void Ocean::Render(Texture * depthTex, Texture * colorTex)
    {
        if (_isUnderWater())
            ;//_renderUnderWater(depthTex, colorTex);
        else
            _renderUpWater(depthTex, colorTex);
    }

	void Ocean::Resize(int w, int h)
	{
		if (mRenderContext != NULL)
		{
			w /= 2;
			h /= 2;

			mRenderTarget->Resize(w, h);
			mDepthBuffer->Resize(w, h);

			mRenderContext->SetViewport(Viewport(0, 0, w, h));
		}
	}

	bool Ocean::_isUnderWater()
	{
		const Float3 & pos = World::Instance()->MainCamera()->GetWorldPosition();

		return pos.y < mPosition.y;
	}

    void Ocean::_renderUpWater(Texture * depthTex, Texture * colorTex)
    {
		RenderSystem * render = RenderSystem::Instance();
		Camera * cam = World::Instance()->MainCamera();
		float time = Root::Instance()->GetTime();

		const Float3 * viewCorner = cam->GetViewCorner();
		const Float3 * worldCorner = cam->GetWorldCorner();
		const Float3 & camPos = cam->GetPosition();

		Float3 worldCornerLeftTop = worldCorner[4] - camPos;
		Float3 worldCornerRightDir = worldCorner[5] - worldCorner[4];
		Float3 worldCornerDownDir = worldCorner[6] - worldCorner[4];

		Float3 viewCornerLeftTop = viewCorner[4];
		Float3 viewCornerRightDir = viewCorner[5] - viewCorner[4];
		Float3 viewCornerDownDir = viewCorner[6] - viewCorner[4];

		Float3 sunDir = RenderFX::Instance()->GetEvParam()->SunDir;
		Float3 sunColor = RenderFX::Instance()->GetEvParam()->SunColor;
		Float3 deepColor = RenderFX::Instance()->GetEvParam()->OceanParam.DeepColor;

		bool hr = false;
		ShaderPass * pass = mTech->GetPass(0);

		hr = pass->SetConst("gCameraPos", camPos);

		hr = pass->SetConst("gWorldCornerLeftTop", worldCornerLeftTop);
		hr = pass->SetConst("gWorldCornerRightDir", worldCornerRightDir);
		hr = pass->SetConst("gWorldCornerDownDir", worldCornerDownDir);

		hr = pass->SetConst("gViewCornerLeftTop", viewCornerLeftTop);
		hr = pass->SetConst("gViewCornerRightDir", viewCornerRightDir);
		hr = pass->SetConst("gViewCornerDownDir", viewCornerDownDir);

		hr = pass->SetConst("gDeepColor", deepColor);
		hr = pass->SetConst("gRefractionDist", 400, 0, 0, 0);

		hr = pass->SetConst("gSunColor", sunColor);
		hr = pass->SetConst("gSunDir", -sunDir);
		hr = pass->SetConst("gSunLightParam", 0.8f, 200, 0, 0);

		float uvNoiseScroll = time * 0.02f;
		hr = pass->SetConst("gNoiseUVParam", 0.001f, 0.001f, uvNoiseScroll, uvNoiseScroll);
		hr = pass->SetConst("gNoiseScale", 0.03f, 0, 0 , 0);

		Float4 uvNormalParam[4] = {
			Float4 (0.0020f, 0.0020f, time * 0.03f, time * 0.03f),
			Float4 (0.0010f, 0.0010f, time * 0.03f, time * 0.03f),
			Float4 (0.0015f, 0.0015f, time * 0.03f, time * 0.03f),
			Float4 (0.0005f, 0.0005f, time * 0.03f, time * 0.03f)
		};

		hr = pass->SetConst("gNormalUVParam", uvNormalParam, 4);
		hr = pass->SetConst("gNormalWeight", 0.08f, 0.1f, 0.06f, 0.1f);

		hr = pass->SetConst("gFarClip", cam->GetFarClip(), 0, 0, 0);

		render->SetTexture(0, mTex_Wave.c_ptr());
		render->SetTexture(1, depthTex);
		render->SetTexture(2, colorTex);
		render->SetTexture(3, mRenderTarget->GetTexture().c_ptr());
		render->SetTexture(4, mTex_Fresnel.c_ptr());

		render->SetTexture(5, mTex_Wave.c_ptr());
		render->SetTexture(6, mTex_Wave.c_ptr());

		render->Render(mTech, mProjGrid->GetRenderOp());
    }

    void Ocean::_renderUnderWater(Texture * depthTex, Texture * colorTex)
    {
		float elapsedTime = Root::Instance()->GetFrameTime();

		mProjGrid->Update(elapsedTime);

        Camera * cam = World::Instance()->MainCamera();
        RenderSystem * render = RenderSystem::Instance();
        float time =  Root::Instance()->GetTime();

        FX_Uniform * uPosition = mTech_UnderWater->GetPass(0)->GetUniform("gPosition");
        FX_Uniform * uScale = mTech_UnderWater->GetPass(0)->GetUniform("gScale");
        FX_Uniform * uUVParam = mTech_UnderWater->GetPass(0)->GetUniform("gUVParam");

        FX_Uniform * uTintColor = mTech_UnderWater->GetPass(0)->GetUniform("gTintColor");
        FX_Uniform * uNoiseScale = mTech_UnderWater->GetPass(0)->GetUniform("gNoiseScale");
        FX_Uniform * uFogParam = mTech_UnderWater->GetPass(0)->GetUniform("gFogParam");
        FX_Uniform * uFogColor = mTech_UnderWater->GetPass(0)->GetUniform("gFogColor");
        FX_Uniform * uCameraPos = mTech_UnderWater->GetPass(0)->GetUniform("gCameraPos");

        float uvScroll = time * 0.02f;
        float fogStart = RenderFX::Instance()->GetEvParam()->OceanParam.FogStart;
        float fogEnd = RenderFX::Instance()->GetEvParam()->OceanParam.FogEnd;
        Float3 fogColor = RenderFX::Instance()->GetEvParam()->OceanParam.FogColor;

        Float3 camPos = cam->GetPosition();

        uPosition->SetConst(0, 0, 0, 0);
        uScale->SetConst(1, 1, 1, 1);
        uUVParam->SetConst(0.01f, 0.01f, uvScroll, uvScroll);

        uTintColor->SetConst(0.1f, 0.1f, 0.3f, 1);
        uNoiseScale->SetConst(0.08f, 0, 0 , 0);
		uFogParam->SetConst(fogStart, fogEnd, 1 / (fogEnd - fogStart), 0.3f);
		uFogColor->SetConst(fogColor.r, fogColor.g, fogColor.b, 0);
		uCameraPos->SetConst(camPos.x, camPos.y, camPos.z, 0);

		render->SetTexture(0, mTex_Wave.c_ptr());
		render->SetTexture(1, colorTex);
		render->SetTexture(2, mTex_Fresnel.c_ptr());

		render->Render(mTech_UnderWater, mProjGrid->GetRenderOp());
	}

	void Ocean::_renderClipPlane()
	{
	}

}