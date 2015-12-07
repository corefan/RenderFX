#pragma once

#include "MRenderFXEntry.h"

namespace Rad {

	class RenderPipelineReflection : public RenderPipeline
	{
	public:
		virtual ~RenderPipelineReflection() {}

		virtual void DoRender()
		{
			mRenderQueue.Clear();

			RenderContext * pRenderContext = World::Instance()->GetCurrentRenderContext();
			Camera * pCamera = World::Instance()->MainCamera();
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

			Mat4 matRefl;
			matRefl.MakeReflection(mPlane);

			Mat4 matView = matRefl * pCamera->GetViewTM();
			Mat4 matProj = pCamera->GetProjTM();

			Mat4 matClip;
			matClip.MakeClipProjection(mPlane, matView * matProj);

			matProj *= matClip;

			RenderSystem::Instance()->SetViewTM(matView);
			RenderSystem::Instance()->SetProjTM(matProj);

			RenderSystem::Instance()->SetClipPlane(pCamera->GetNearClip(), pCamera->GetFarClip());
			RenderSystem::Instance()->SetTime(Root::Instance()->GetTime());

			RenderSystem::Instance()->SetLight(World::Instance()->MainLight());

			World::Instance()->E_RenderPrepare();

			World::Instance()->E_RenderSolidBegin();

			do 
			{
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

				for (int i = 0; i < arr.Size(); ++i)
				{
					RenderSystem::Instance()->Render(arr[i]->GetCurrentShaderFX(), arr[i]);
				}

			} while (0);

			World::Instance()->E_RenderSolid();

			World::Instance()->E_RenderSolidEnd();

			World::Instance()->E_RenderAlphaBegin();

			do 
			{
				Array<RenderObject *> & arr = mRenderQueue.GetAlphaObjects();

				if (arr.Size() > 0)
				{
					AlphaSorter sorter(pCamera);
					Sort(&arr[0], arr.Size(), sorter);
				}

				for (int i = 0; i < arr.Size(); ++i)
				{
					pShaderProvider->ApplyShaderFX(arr[i]);

					eBlendMode bm = arr[i]->GetMaterial()->blendMode;
					if (bm == eBlendMode::OPACITY || bm == eBlendMode::ALPHA_TEST)
					{
						RenderSystem::Instance()->SetColorWriteEnable(false);
						RenderSystem::Instance()->Render(arr[i]->GetCurrentShaderFX(), arr[i]);
						RenderSystem::Instance()->SetColorWriteEnable(true);
					}

					RenderSystem::Instance()->Render(arr[i]->GetCurrentShaderFX(), arr[i]);
				}

			} while(0);

			World::Instance()->E_RenderAlpha();

			World::Instance()->E_RenderAlphaEnd();
		}

		void SetPlane(const Plane & plane)
		{ 
			mPlane = plane;
		}

		bool _inVisible(const Aabb & bound)
		{
			Float3 center = bound.GetCenter();
			Float3 half = bound.GetHalfSize();

			Plane::Side side = mPlane.AtSide(center, half);

			return side != Plane::NEGATIVE;
		}

	protected:
		Plane mPlane;
	};

}

