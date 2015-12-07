#include "MLighting.h"
#include "MRenderFX.h"

namespace Rad {

	Lighting::Lighting()
	{
		int w = RenderFX::Instance()->GetColorBuffer()->GetWidth();
		int h = RenderFX::Instance()->GetColorBuffer()->GetHeight();

		mRenderTarget = HWBufferManager::Instance()->NewRenderTarget(w, h, ePixelFormat::R16G16B16A16F);

		mDirLightFX = ShaderFXManager::Instance()->Load("RenderFX.DirLight", "RenderFX/Lighting.mfx", 
			"#define N_LIGHTS 1 \n #define DIRECTION \n \n");

		mPointLightFX[0] = ShaderFXManager::Instance()->Load("RenderFX.PointLight1", "RenderFX/Lighting.mfx", 
			"#define N_LIGHTS 1 \n #define POINT \n");
		mPointLightFX[1] = ShaderFXManager::Instance()->Load("RenderFX.PointLight2", "RenderFX/Lighting.mfx", 
			"#define N_LIGHTS 2 \n #define POINT \n");
		mPointLightFX[2] = ShaderFXManager::Instance()->Load("RenderFX.PointLight3", "RenderFX/Lighting.mfx", 
			"#define N_LIGHTS 3 \n #define POINT \n");
		mPointLightFX[3] = ShaderFXManager::Instance()->Load("RenderFX.PointLight4", "RenderFX/Lighting.mfx", 
			"#define N_LIGHTS 4 \n #define POINT \n");

		mSpotLightFX[0] = ShaderFXManager::Instance()->Load("RenderFX.SpotLight1", "RenderFX/Lighting.mfx", 
			"#define N_LIGHTS 1 \n #define SPOT \n");
		mSpotLightFX[1] = ShaderFXManager::Instance()->Load("RenderFX.SpotLight2", "RenderFX/Lighting.mfx", 
			"#define N_LIGHTS 2 \n #define SPOT \n");
		mSpotLightFX[2] = ShaderFXManager::Instance()->Load("RenderFX.SpotLight3", "RenderFX/Lighting.mfx", 
			"#define N_LIGHTS 3 \n #define SPOT \n");
		mSpotLightFX[3] = ShaderFXManager::Instance()->Load("RenderFX.SpotLight4", "RenderFX/Lighting.mfx", 
			"#define N_LIGHTS 4 \n #define SPOT \n");
	}

	Lighting::~Lighting()
	{
	}

	void Lighting::Resize(int w, int h)
	{
		mRenderTarget->Resize(w, h);
	}

	void Lighting::Render(Texture * normalTex, Texture * depthTex, Texture * shadowTex)
	{
		profile_code();

		RenderSystem::Instance()->SetRenderTarget(0, mRenderTarget.c_ptr());
		RenderSystem::Instance()->PrepareRendering();

		RenderSystem::Instance()->SetViewport(0, 0, mRenderTarget->GetWidth(), mRenderTarget->GetHeight());

		RenderSystem::Instance()->Clear(eClearMode::COLOR, Float4(0, 0, 0), 1, 0);

		RenderSystem::Instance()->SetTexture(0, normalTex);
		RenderSystem::Instance()->SetTexture(1, depthTex);
		RenderSystem::Instance()->SetTexture(2, shadowTex);

		if (1)
		{
			Light * l = World::Instance()->MainLight();
			ShaderFX * fx = mDirLightFX;

			const Mat4 & matView = World::Instance()->MainCamera()->GetViewTM();
			Float3 lightPos = l->GetWorldPosition();
			Float3 lightDir = -l->GetWorldRotation().GetDirVector();
			Float4 lightAttenParam = l->GetAttenParam();
			Float4 lightSpotParam = l->GetSpotParam();
			Float3 lightDiffuse = l->GetDiffuse();
			Float3 lightSpecular = l->GetSpecular();

			lightPos.TransformA(matView);
			lightDir.TransformN(matView);

			const Float3 * corner = World::Instance()->MainCamera()->GetViewCorner();
			Float3 cornerLeftTop = corner[4];
			Float3 cornerRightDir = corner[5] - corner[4];
			Float3 cornerDownDir = corner[6] - corner[4];

			FX_Uniform * uCornerLeftTop = fx->GetPass(0)->GetUniform("gCornerLeftTop");
			FX_Uniform * uCornerRightDir = fx->GetPass(0)->GetUniform("gCornerRightDir");
			FX_Uniform * uCornerDownDir = fx->GetPass(0)->GetUniform("gCornerDownDir");
			FX_Uniform * uLightPos = fx->GetPass(0)->GetUniform("gLightPos");
			FX_Uniform * uLightDir = fx->GetPass(0)->GetUniform("gLightDir");
			FX_Uniform * uLightAttenParam = fx->GetPass(0)->GetUniform("gLightAttenParam");
			FX_Uniform * uLightSpotParam = fx->GetPass(0)->GetUniform("gLightSpotParam");
			FX_Uniform * uDiffuse = fx->GetPass(0)->GetUniform("gDiffuse");
			FX_Uniform * uSpecular = fx->GetPass(0)->GetUniform("gSpecular");

			if (uCornerLeftTop)
			{
				uCornerLeftTop->SetConst(cornerLeftTop.x, cornerLeftTop.y, cornerLeftTop.z, 0);
				uCornerRightDir->SetConst(cornerRightDir.x, cornerRightDir.y, cornerRightDir.z, 0);
				uCornerDownDir->SetConst(cornerDownDir.x, cornerDownDir.y, cornerDownDir.z, 0);
			}
			
			//uLightPos->SetConst(lightPos.x, lightPos.y, lightPos.z, 0);
			uLightDir->SetConst(lightDir.x, lightDir.y, lightDir.z, 0);
			//uLightAttenParam->SetConst(lightAttenParam);
			//uLightSpotParam->SetConst(lightSpotParam);
			uDiffuse->SetConst(lightDiffuse.r, lightDiffuse.g, lightDiffuse.b, 1);

			if (uSpecular)
				uSpecular->SetConst(lightSpecular.r, lightSpecular.g, lightSpecular.b, 1);

			RenderSystem::Instance()->RenderScreenQuad(fx);
		}

		RenderSystem::Instance()->SetTexture(2, RenderHelper::Instance()->GetWhiteTexture().c_ptr());

		FixedArray<Light *, 4> lights;
		VisibleCullerPtr vbc = RenderFX::Instance()->GetContext()->GetVisibleCuller();

		for (int i = 0; i < vbc->GetLightArray().Size(); ++i)
		{
			Light * l = vbc->GetLightArray().At(i);
			if (l->GetType() == eLightType::POINT)
			{
				lights.PushBack(l);

				if (lights.Size() == N_LIGHTS)
				{
					_render(lights);
					lights.Clear();
				}
			}
		}

		if (lights.Size() > 0)
		{
			_render(lights);
			lights.Clear();
		}
	}

	void Lighting::_render(const FixedArray<Light *, N_LIGHTS> & lights)
	{
		eLightType type = lights[0]->GetType();
		int nlights = lights.Size();

		const Mat4 & matView = World::Instance()->MainCamera()->GetViewTM();
		Float4 lightPos[N_LIGHTS];
		Float4 lightDir[N_LIGHTS];
		Float4 lightAttenParam[N_LIGHTS];
		Float4 lightSpotParam[N_LIGHTS];
		Float4 lightDiffuse[N_LIGHTS];
		Float4 lightSpecular[N_LIGHTS];

		for (int i = 0; i < nlights; ++i)
		{
			Light * l = lights[i];

			lightPos[i] = l->GetWorldPosition();
			lightDir[i] = -l->GetWorldRotation().GetDirVector();
			lightAttenParam[i] = l->GetAttenParam();
			lightSpotParam[i] = l->GetSpotParam();
			lightDiffuse[i] = l->GetDiffuse();
			lightSpecular[i] = l->GetSpecular();

			lightPos[i].Transform(matView);
			lightDir[i].TransformN(matView);
		}

		const Float3 * corner = World::Instance()->MainCamera()->GetViewCorner();
		Float3 cornerLeftTop = corner[4];
		Float3 cornerRightDir = corner[5] - corner[4];
		Float3 cornerDownDir = corner[6] - corner[4];

		ShaderFX * fx = type == eLightType::POINT ? mPointLightFX[nlights - 1] : mSpotLightFX[nlights - 1];

		FX_Uniform * uCornerLeftTop = fx->GetPass(0)->GetUniform("gCornerLeftTop");
		FX_Uniform * uCornerRightDir = fx->GetPass(0)->GetUniform("gCornerRightDir");
		FX_Uniform * uCornerDownDir = fx->GetPass(0)->GetUniform("gCornerDownDir");
		FX_Uniform * uLightPos = fx->GetPass(0)->GetUniform("gLightPos");
		FX_Uniform * uLightDir = fx->GetPass(0)->GetUniform("gLightDir");
		FX_Uniform * uLightAttenParam = fx->GetPass(0)->GetUniform("gLightAttenParam");
		FX_Uniform * uLightSpotParam = fx->GetPass(0)->GetUniform("gLightSpotParam");
		FX_Uniform * uDiffuse = fx->GetPass(0)->GetUniform("gDiffuse");
		FX_Uniform * uSpecular = fx->GetPass(0)->GetUniform("gSpecular");

		uCornerLeftTop->SetConst(cornerLeftTop.x, cornerLeftTop.y, cornerLeftTop.z, 0);
		uCornerRightDir->SetConst(cornerRightDir.x, cornerRightDir.y, cornerRightDir.z, 0);
		uCornerDownDir->SetConst(cornerDownDir.x, cornerDownDir.y, cornerDownDir.z, 0);

		uLightPos->SetConst(lightPos, nlights);
		uLightDir->SetConst(lightDir, nlights);
		uLightAttenParam->SetConst(lightAttenParam, nlights);
		if (uLightSpotParam) 
			uLightSpotParam->SetConst(lightSpotParam, nlights);
		
		uDiffuse->SetConst(lightDiffuse, nlights);
		if (uSpecular) 
			uSpecular->SetConst(lightSpecular, nlights);

		RenderSystem::Instance()->RenderScreenQuad(fx);
	}

}