#include "MShadow.h"
#include "MRoot.h"
#include "MFunctional.h"
#include "MRenderSystem.h"
#include "MRenderFX.h"

namespace Rad {

	Shadow::Shadow()
		: mShadowCamera(NULL)
		, mShadowContext(NULL)
	{
		mDist[0] = 30 * UNIT_METRES;
		mDist[1] = 60 * UNIT_METRES;
		mDist[2] = 90 * UNIT_METRES;

		mBias[0] = 0.005f;
		mBias[1] = 0.010f;
		mBias[2] = 0.015f;

		mOffset = 100 * UNIT_METRES;

		mMapSize = 2048;
		mFadeRatio = 0.8f;

		//
		int w = RenderFX::Instance()->GetColorBuffer()->GetWidth();
		int h = RenderFX::Instance()->GetColorBuffer()->GetHeight();

		mShadowCamera = new Camera;
		mRT_Depth = HWBufferManager::Instance()->NewRenderTarget(mMapSize, mMapSize, ePixelFormat::R32F);
		mDepthBuffer = HWBufferManager::Instance()->NewDepthBuffer(mMapSize, mMapSize, ePixelFormat::D24S8);
		mRT_Shadow = HWBufferManager::Instance()->NewRenderTarget(w, h, ePixelFormat::R8G8B8);

		mShadowContext = new RenderContext(0, "ShadowContext");
		mShadowContext->SetCamera(mShadowCamera);
		mShadowContext->SetRenderTarget(0, mRT_Depth);
		mShadowContext->SetDepthBuffer(mDepthBuffer);
		mShadowContext->SetVisibleCuller(new ShadowVisibleCuller(this));
		mShadowContext->SetRenderPipeline(new ShadowRenderPipline(this));
		mShadowContext->SetColorClear(eClearMode::NONE, Float4(1, 1, 1, 1));

		mTech_ShadowDepth = ShaderFXManager::Instance()->Load("RenderFX.ShadowDepth", "RenderFX/ShadowDepth.mfx");
		mTech_ShadowDepthSkined = ShaderFXManager::Instance()->Load("RenderFX.ShadowDepthSkined", "RenderFX/ShadowDepth.mfx", "#define D_SKINED/n");
		mTech_Shadow[0] = ShaderFXManager::Instance()->Load("RenderFX.ShadowL0", "RenderFX/Shadow.mfx", "#define D_LAYER0\n");
		mTech_Shadow[1] = ShaderFXManager::Instance()->Load("RenderFX.ShadowL1", "RenderFX/Shadow.mfx", "#define D_LAYER1\n");
		mTech_Shadow[2] = ShaderFXManager::Instance()->Load("RenderFX.ShadowL2", "RenderFX/Shadow.mfx", "#define D_LAYER2\n");

		mTex_Random = HWBufferManager::Instance()->LoadTexture("RenderFX/random.png");
	}

	Shadow::~Shadow()
	{
		safe_delete (mShadowContext);
		safe_delete (mShadowCamera);
	}

	void Shadow::SetMapSize(int mapSize)
	{
		mMapSize = mapSize;
	}

	void Shadow::SetDistance(int cascaded, float distance)
	{
		mDist[cascaded] = distance;
	}

	void Shadow::SetFadeRatio(float fadeRatio)
	{
		mFadeRatio = fadeRatio;
	}

	void Shadow::SetOffset(float offset)
	{
		mOffset = offset;
	}

	bool Shadow::InVisible(int layer, const Aabb & bound)
	{
		const Plane * planes = (const Plane *)&mCascadedView[layer].mFrustum;
		Float3 center = bound.GetCenter();
		Float3 half = bound.GetHalfSize();

		for (int i = 0; i < 6; ++i)
		{
			Plane::Side side = planes[i].AtSide(center, half);

			if (side == Plane::NEGATIVE)
				return false;
		}

		return true;
	}

	void Shadow::Render(Texture * depthTex)
	{
		profile_code();

		mDepthTexture = depthTex;

		_updateCamera();

		World::Instance()->BeginRenderContext(mShadowContext);
		mShadowContext->DoRender(World::Instance()->FrameId());
		World::Instance()->EndRenderContext();
	}

	Mat4 Shadow::_calcuCropMatrix(int layer)
	{
		Float3 frustum[8];

		float nearClip = mDist[layer];
		float farClip = mDist[layer + 1];

		World::Instance()->MainCamera()->GetWorldCorner(frustum, nearClip, farClip);
		Aabb cropBB = _calcuAabb(frustum);

		cropBB.minimum.z = 0.0f; 

		float scaleX, scaleY;
		float offsetX, offsetY;
		scaleX = 2.0f / (cropBB.maximum.x - cropBB.minimum.x);
		scaleY = 2.0f / (cropBB.maximum.y - cropBB.minimum.y);
		offsetX = -cropBB.minimum.x * scaleX - 1;
		offsetY = -cropBB.minimum.y * scaleY - 1;

		return Mat4(scaleX,		0,			0, 0,
					0,			scaleY,     0, 0,
					0,			0,			1, 0,
					offsetX,	offsetY,	0, 1);
	}

	void Shadow::_calcuCascadedView(int layer)
	{
		Camera * worldCam = World::Instance()->MainCamera();
		Light * light = World::Instance()->MainLight();

		float nearClip, farClip = mDist[layer];

		if (layer == 0)
			nearClip = worldCam->GetNearClip();
		else
			nearClip = mDist[layer - 1];

		Float3 xAixs = worldCam->GetWorldRotation().GetRightVector();
		Float3 yAixs = worldCam->GetWorldRotation().GetUpVector();
		Float3 zAixs = light->GetWorldRotation().GetDirVector();

		if (Math::Abs(zAixs.Dot(yAixs)) > 0.99f)
		{
			yAixs = Float3::CrossN(zAixs, xAixs);
			xAixs = Float3::CrossN(yAixs, zAixs);
		}
		else
		{
			xAixs = Float3::CrossN(yAixs, zAixs);
			yAixs = Float3::CrossN(zAixs, xAixs);
		}

		if (xAixs.Dot(worldCam->GetWorldRotation().GetDirVector()) < 0)
			xAixs = -xAixs;

		yAixs = Float3::CrossN(zAixs, xAixs);

		Mat4 matView, matProj;
		Quat qOrient;

		qOrient.FromAxis(xAixs, yAixs, zAixs);
		matView.MakeViewLH(worldCam->GetPosition(), qOrient);

		// calculate aabb
		Float3 corner[8], t_corner[8];
		Aabb aabb = Aabb::Invalid;

		worldCam->GetWorldCorner(t_corner, nearClip, farClip);
		for (int i = 0; i < 8; ++i)
		{
			corner[i] = t_corner[i] * matView;
		}

		for (int i = 0; i < 8; ++i)
		{
			aabb.minimum = Float3::Minimum(aabb.minimum, corner[i]);
			aabb.maximum = Float3::Maximum(aabb.maximum, corner[i]);
		}

		Float3 center = aabb.GetCenter();
		Float3 size = aabb.GetSize();

		matView.Inverse();
		center *= matView;

		Float3 lightPos = center - zAixs * mOffset;

		matView.MakeViewLH(lightPos, qOrient);
		matProj.MakeOrthoLH(size.x, size.y, nearClip, mOffset + size.z);

		mCascadedView[layer].mView = matView;
		mCascadedView[layer].mProj = matProj;
		mCascadedView[layer].mViewProj = matView * matProj;
		mCascadedView[layer].mFrustum.FromMatrix(mCascadedView[layer].mViewProj);
	}

	Aabb Shadow::_calcuAabb(const Float3 * v)
	{
		Aabb bound = Aabb::Invalid;

		const Mat4 & matViewProj = mShadowCamera->GetViewProjTM();

		for (int i = 0; i < 8; ++i)
		{
			Float3 pv = v[i] * matViewProj;

			bound.minimum = Float3::Minimum(bound.minimum, pv);
			bound.maximum = Float3::Maximum(bound.maximum, pv);
		}

		return bound;
	}

	void Shadow::_updateCamera()
	{
		Camera * worldCam = World::Instance()->MainCamera();
		Light * light = World::Instance()->MainLight();

		float nearClip = worldCam->GetNearClip();
		float farClip = mDist[K_NumShadowLayers - 1];

		Float3 xAixs = worldCam->GetWorldRotation().GetRightVector();
		Float3 yAixs = worldCam->GetWorldRotation().GetUpVector();
		Float3 zAixs = light->GetWorldRotation().GetDirVector();

		if (Math::Abs(zAixs.Dot(yAixs)) > 0.99f)
		{
			yAixs = Float3::CrossN(zAixs, xAixs);
			xAixs = Float3::CrossN(yAixs, zAixs);
		}
		else
		{
			xAixs = Float3::CrossN(yAixs, zAixs);
			yAixs = Float3::CrossN(zAixs, xAixs);
		}

		if (xAixs.Dot(worldCam->GetWorldRotation().GetDirVector()) < 0)
			xAixs = -xAixs;

		yAixs = Float3::CrossN(zAixs, xAixs);

		Quat qOrient;
		qOrient.FromAxis(xAixs, yAixs, zAixs);

		Mat4 matView;
		matView.MakeViewLH(worldCam->GetPosition(), qOrient);

		// calculate aabb
		Float3 corner[8], t_corner[8];
		Aabb aabb = Aabb::Invalid;

		worldCam->GetWorldCorner(t_corner, nearClip, farClip);
		for (int i = 0; i < 8; ++i)
		{
			corner[i] = t_corner[i] * matView;
		}

		for (int i = 0; i < 8; ++i)
		{
			aabb.minimum = Float3::Minimum(aabb.minimum, corner[i]);
			aabb.maximum = Float3::Maximum(aabb.maximum, corner[i]);
		}

		Float3 center = aabb.GetCenter();
		Float3 size = aabb.GetSize();

		matView.Inverse();
		center *= matView;

		Float3 lightPos = center - zAixs * mOffset;

		mShadowCamera->SetPosition(lightPos);
		mShadowCamera->SetRotation(qOrient);
		mShadowCamera->SetClipPlane(nearClip, mOffset + size.z);
		mShadowCamera->SetOrthoParam(size.x, size.y, true);
	}

	void Shadow::_applyShaderFX(RenderObject * able, int flag)
	{
		if (!able->IsSkined())
		{
			able->SetCurrentShaderFX(mTech_ShadowDepth);
		}
		else
		{
			able->SetCurrentShaderFX(mTech_ShadowDepthSkined);
		}
	}
	
	void Shadow::_genShadowMap(int layer)
	{
		RenderSystem::Instance()->SetRenderTarget(0, mRT_Shadow.c_ptr());
		RenderSystem::Instance()->SetRenderTarget(1, NULL);
		RenderSystem::Instance()->SetRenderTarget(2, NULL);
		RenderSystem::Instance()->SetRenderTarget(3, NULL);
		RenderSystem::Instance()->SetDepthBuffer(NULL);
		RenderSystem::Instance()->PrepareRendering();

		RenderSystem::Instance()->SetViewport(0, 0, mRT_Shadow->GetWidth(), mRT_Shadow->GetHeight());

		if (layer == 0)
			RenderSystem::Instance()->Clear(eClearMode::COLOR, Float4(1, 1, 1, 1), 1, 0);

		Camera * cam = World::Instance()->MainCamera();
		const Float3 * corner = cam->GetViewCorner();

		Mat4 matInverseView = cam->GetViewTM();
		matInverseView.Inverse();

		Float3 cornerLeftTop = corner[4];
		Float3 cornerRightDir = corner[5] - corner[4];
		Float3 cornerDownDir = corner[6] - corner[4];

		Float4 shadowInfo = Float4::Zero;

		if (layer == 0)
			shadowInfo.x = cam->GetNearClip();
		else
			shadowInfo.x = mDist[layer - 1];

		shadowInfo.y = mDist[layer];
		shadowInfo.z = mBias[layer];

		ShaderFX * fx = mTech_Shadow[layer];

		FX_Uniform * uShadowInfo = fx->GetPass(0)->GetUniform("gShadowInfo");
		FX_Uniform * uMatShadow = fx->GetPass(0)->GetUniform("matShadow");
		FX_Uniform * uCornerLeftTop = fx->GetPass(0)->GetUniform("gCornerLeftTop");
		FX_Uniform * uCornerRightDir = fx->GetPass(0)->GetUniform("gCornerRightDir");
		FX_Uniform * uCornerDownDir = fx->GetPass(0)->GetUniform("gCornerDownDir");

		uCornerLeftTop->SetConst(cornerLeftTop.x, cornerLeftTop.y, cornerLeftTop.z, 0);
		uCornerRightDir->SetConst(cornerRightDir.x, cornerRightDir.y, cornerRightDir.z, 0);
		uCornerDownDir->SetConst(cornerDownDir.x, cornerDownDir.y, cornerDownDir.z, 0);

		uShadowInfo->SetConst(shadowInfo.x, shadowInfo.y, shadowInfo.z, 0);
		uMatShadow->SetConst(matInverseView * mCascadedView[layer].mViewProj);

		RenderSystem::Instance()->SetTexture(0, mDepthTexture);
		RenderSystem::Instance()->SetTexture(1, mRT_Depth->GetTexture().c_ptr());
		RenderSystem::Instance()->SetTexture(2, mTex_Random.c_ptr());

		RenderSystem::Instance()->RenderScreenQuad(fx);
	}

	//
	ShadowVisibleCuller::ShadowVisibleCuller(Shadow * shadow)
	{
		mShadow = shadow;
	}

	void ShadowVisibleCuller::DoCull(int frameId, Camera * camera)
	{
		if (mFrameId == frameId)
			return ;

		mFrameId = frameId;

		mNodeArray.Clear();
		mLightArray.Clear();

		World::Instance()->MainZone()->ImpVisiblityCull(mNodeArray, camera);

		mShadow->E_VisibleCull(camera);
	}

	//
	ShadowRenderPipline::ShadowRenderPipline(Shadow * shadow)
		: mShadow(shadow)
	{
	}

	void ShadowRenderPipline::DoRender()
	{
		RenderContext * pRenderContext = mShadow->GetShadowContext();
		Camera * pCamera = pRenderContext->GetCamera();
		Array<Node*> & visibleArray = pRenderContext->GetVisibleCuller()->GetNodeArray();

		for (int i = 0; i < Shadow::K_NumShadowLayers; ++i)
		{
			mRenderQueue.Clear();

			mShadow->_calcuCascadedView(i);

			for (int j = 0; j < visibleArray.Size(); ++j)
			{
				Node * n = visibleArray[j];
				if (TYPE_OF(Mesh, n) && mShadow->InVisible(i, n->GetWorldAabb()))
				{
					n->AddRenderQueue(&mRenderQueue);
				}
			}

			RenderSystem::Instance()->SetRenderTarget(0, pRenderContext->GetRenderTarget(0).c_ptr());
			RenderSystem::Instance()->SetDepthBuffer(pRenderContext->GetDepthBuffer().c_ptr());
			RenderSystem::Instance()->PrepareRendering();

			RenderSystem::Instance()->SetViewport(pRenderContext->GetViewport());

			RenderSystem::Instance()->Clear(eClearMode::ALL, Float4(1, 1, 1), 1, 0);

			const Shadow::CascadedView & view = mShadow->GetCascadedView(i);

			RenderSystem::Instance()->SetViewTM(view.mView);
			RenderSystem::Instance()->SetProjTM(view.mProj);

			RenderSystem::Instance()->SetClipPlane(pCamera->GetNearClip(), pCamera->GetFarClip());
			RenderSystem::Instance()->SetTime(Root::Instance()->GetTime());

			do 
			{
				Array<RenderObject *> & arr = mRenderQueue.GetSolidObjects();

				for (int i = 0; i < arr.Size(); ++i)
				{
					mShadow->_applyShaderFX(arr[i], 0);
				}

				if (arr.Size() > 0)
				{
					SolidSorter sorter;
					Sort(&arr[0], arr.Size(), sorter);
				}

				RenderSystem::Instance()->SetLight(World::Instance()->MainLight());

				for (int i = 0; i < arr.Size(); ++i)
				{
					RenderSystem::Instance()->Render(arr[i]->GetCurrentShaderFX(), arr[i]);
				}

			} while (0);

			mShadow->E_RenderDepth(i);

			mShadow->_genShadowMap(i);
		}
	}

}

