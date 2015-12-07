/*
*	WaterMeshShader
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "MRenderFXEntry.h"

namespace Rad {

	class WaterMesh;

	class RFX_ENTRY WaterMeshShader : public IObject, public RenderCallBack
	{
		DECLARE_REF();
		DECLARE_RTTI();

	public:
		WaterMeshShader();
		virtual ~WaterMeshShader();

		void
			Release() { delete this; }

		virtual void
			Update(WaterMesh * water, float frameTime) = 0;
	};

	typedef SmartPtr<WaterMeshShader> WaterMeshShaderPtr;

	//
	class RFX_ENTRY WaterMeshShaderStandard : public WaterMeshShader
	{
		DECLARE_RTTI();
		DECLARE_PROPERTY(WaterMeshShader);

	protected:
		float mUVScale;
		Float4 mColor;

	public:
		WaterMeshShaderStandard();
		virtual ~WaterMeshShaderStandard();

		void
			SetUVScale(float uvScale);
		float
			GetUVScale() { return mUVScale; }

		void
			SetColor(const Float4 & color);
		const Float4 &
			GetColor() { return mColor; }

		virtual void
			Update(WaterMesh * water, float frameTime) {}
		virtual void
			OnCallBack(RenderObject * obj);

	protected:
		ShaderFX * mShaderFX;
	};

}