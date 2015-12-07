/*
*	EvParam
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "MRenderFXEntry.h"

namespace Rad {

	struct RFX_ENTRY eEvKeyType : public IEnum
	{
		DECLARE_ENUM(eEvKeyType);

		enum enum_t
		{
			NIGHT,
			MORNING,
			NOON,
			EVENING,

			// Special KeyFrame, you can rename it.
			SPECIAL0,
			SPECIAL1,
			SPECIAL2,
			SPECIAL3,
			SPECIAL4,
			SPECIAL5,
			SPECIAL6,
			SPECIAL7,
			SPECIAL8,
			SPECIAL9,

			MAX,
			UNKNOWN,
		};

		M_ENUMERATION(eEvKeyType);
	};

	struct EvHdrParam
	{
		EvHdrParam();

		float Exposure;
		float Threshold;
		Float3 Weights;
		float SharpScale;
		float SharpFadeStart;
		float SharpFadeEnd;
	};

	struct EvGodRayParam
	{
		EvGodRayParam();

		float SunLum;
		float SunInner;
		float SunPower;
		float SunSize;
		float UVStep;
		float Weight;
	};

	struct EvCloudParam
	{
		EvCloudParam();

		float Height;
		float Curved;
		float Mass;
		Float4 Weights;
		Float4 UVScales;
		Float4 UVScrolls;
		float Alpha;
		float AlphaStrength;
		float LightStrength;
		float FadeStart;
		float FadeStrength;
		Float3 Ambient;
		Float3 Diffuse;
		float AmbientScale;
		float DiffuseScale;
	};

	struct EvOceanParam
	{
		EvOceanParam();

		Float3 DeepColor;
		Float3 ReflColor;
		Float3 FogColor;
		float FogStart;
		float FogEnd;
		float RayLum;
		float RayExp0;
		float RayExp1;
	};

	struct EvGrassParam
	{
		EvGrassParam();

		float WaveSpeed;
		float WaveStrength;
		float VisibleRadius;
	};

	struct RFX_ENTRY EvGlobalParam : public IObject
	{
		DECLARE_PROPERTY(IObject);

		EvGlobalParam();

		String SkyTexture;
		String SunTexture;
		String MoonTexture;

		float SkyVOff;
		float SunPitch;
	};

	struct RFX_ENTRY EvKeyFrame : public IObject
	{
		DECLARE_PROPERTY(IObject);

		float Time;
		float StarfieldLum;
		float SkyLum;
		float SunLum;
		float SunSize;
		float SunPower;
		Float3 SunColor;
		float MoonLum;
		float MoonSize;
		float MoonPhase;

		Float3 WindDir;

		Float3 LightAmbient;
		Float3 LightDiffuse;
		Float3 LightSpecular;
		float LightStrength;

		float FogStart;
		float FogEnd;
		Float3 FogColor;

		EvHdrParam HdrParam;
		EvGodRayParam GodRayParam;
		EvGrassParam GrassParam;
		EvCloudParam CloudParam;
		EvOceanParam OceanParam;

		EvKeyFrame();

		static void 
			Lerp(EvKeyFrame & o, const EvKeyFrame & k0, const EvKeyFrame & k1, float t);
	};

	struct RFX_ENTRY EvParam : public IObject
	{
		float StarfieldLum;
		float SkyLum;
		float SkyU;

		Float3 SunPos;
		Float3 SunDir;
		Float3 SunColor;
		float SunLum;
		float SunSize;
		float SunPower;

		Float3 WindDir;

		Float3 MoonPos;
		Float3 MoonDir;
		float MoonLum;
		float MoonSize;
		float MoonPhase;

		Float3 LightDir;
		Float3 LightAmbient;
		Float3 LightDiffuse;
		Float3 LightSpecular;

		float FogStart;
		float FogEnd;
		Float3 FogColor;

		EvHdrParam HdrParam;
		EvGodRayParam GodRayParam;
		EvGrassParam GrassParam;
		EvCloudParam CloudParam;
		EvOceanParam OceanParam;
	};

}
