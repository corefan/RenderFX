#include "MWaterMeshShader.h"
#include "MWaterMesh.h"

namespace Rad {

	ImplementRTTI(WaterMeshShader, IObject);

	WaterMeshShader::WaterMeshShader()
	{
	}

	WaterMeshShader::~WaterMeshShader()
	{
	}

	//
	ImplementRTTI(WaterMeshShaderStandard, WaterMeshShader);

	DF_PROPERTY_BEGIN(WaterMeshShaderStandard)
		DF_PROPERTY(WaterMeshShaderStandard, mUVScale, "Shader", "UVScale", PT_Float)
		DF_PROPERTY(WaterMeshShaderStandard, mColor, "Shader", "Color", PT_Float4)
		DF_PROPERTY_END()

	WaterMeshShaderStandard::WaterMeshShaderStandard()
		: mUVScale(0.05f)
		, mColor(1, 1, 1, 1)
	{
		mShaderFX = ShaderFXManager::Instance()->Load("RenderFX.WaterMesh", "RenderFX/WaterMesh.mfx");
	}

	WaterMeshShaderStandard::~WaterMeshShaderStandard()
	{
	}

	void WaterMeshShaderStandard::SetUVScale(float uvScale)
	{
		mUVScale = uvScale;
	}

	void WaterMeshShaderStandard::SetColor(const Float4 & color)
	{
		mColor = color;
	}

	void WaterMeshShaderStandard::OnCallBack(RenderObject * obj)
	{
		WaterMesh * wm = (WaterMesh *)obj;

		mShaderFX->GetPass(0)->SetConst("u_Param", wm->GetWidth(), wm->GetLength(), mUVScale, 0);
		mShaderFX->GetPass(0)->SetConst("u_Color", mColor);
	}

}