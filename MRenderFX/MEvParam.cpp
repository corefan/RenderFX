#include "MEvParam.h"

namespace Rad {

	DF_ENUM_BEGIN(eEvKeyType)
		DF_ENUM(NIGHT)
		DF_ENUM(MORNING)
		DF_ENUM(NOON)
		DF_ENUM(EVENING)

		DF_ENUM(SPECIAL0)
		DF_ENUM(SPECIAL1)
		DF_ENUM(SPECIAL2)
		DF_ENUM(SPECIAL3)
		DF_ENUM(SPECIAL4)
		DF_ENUM(SPECIAL5)
		DF_ENUM(SPECIAL6)
		DF_ENUM(SPECIAL7)
		DF_ENUM(SPECIAL8)
		DF_ENUM(SPECIAL9)
	DF_ENUM_END()


	EvHdrParam::EvHdrParam()
	{
		Exposure = 2;
		Threshold = 0.8f;
		Weights = Float3(0.1f, 0.3f, 1.0f);

		SharpScale = 0.5f;
		SharpFadeStart = 50 * UNIT_METRES;
		SharpFadeEnd = 100 * UNIT_METRES;
	}

	EvGodRayParam::EvGodRayParam()
	{
		SunLum = 1.8f;
		SunInner = 0;
		SunPower = 1;
		SunSize = 4000;
		UVStep = 48;
		Weight = 0.8f;
	}

	EvCloudParam::EvCloudParam()
	{
		Height = 0.15f;
		Curved = 0.4f;
		Mass = 0.4f;
		Weights = Float4(0.7f, 0.3f, 0, 0);
		UVScales = Float4(1.5f, 2.5f, 0, 0);
		UVScrolls = Float4(0.005f, 0.015f, 0, 0);
		Alpha = 0.25f;
		AlphaStrength = 15;
		LightStrength = 9.0f;
		FadeStart = 0.7f;
		FadeStrength = 1;
		Ambient = Float3(0.4f, 0.66f, 0.84f);
		Diffuse = Float3(0.78f, 0.88f, 1);
		AmbientScale = 0.7f;
		DiffuseScale = 1.5f;
	}

	EvOceanParam::EvOceanParam()
	{
		DeepColor = Float3(18.0f / 255.0f, 22.0f / 255.0f, 36.0f / 255.0f);
		ReflColor = Float3(1, 1, 1);
		FogColor = Float3(29.0f / 255.0f, 57.0f / 255.0f, 68.0f / 255.0f);
		FogStart = 20;
		FogEnd = 120;
		RayLum = 0.005f;
		RayExp0 = 0.50f;
		RayExp1 = 2.49f;
	}

	EvGrassParam::EvGrassParam()
	{
		WaveSpeed = 0.8f;
		WaveStrength = 0.15f;
		VisibleRadius = 2500;
	}

	DF_PROPERTY_BEGIN(EvGlobalParam)
		DF_PROPERTY_EX(EvGlobalParam, SkyTexture, "", "SkyTexture", "PT_Filename", PT_String)
		DF_PROPERTY_EX(EvGlobalParam, MoonTexture, "", "MoonTexture", "PT_Filename", PT_String)
		DF_PROPERTY(EvGlobalParam, SkyVOff, "", "SkyVOff", PT_Float)
		DF_PROPERTY(EvGlobalParam, SunPitch, "", "SunPitch", PT_Float)
	DF_PROPERTY_END()

	EvGlobalParam::EvGlobalParam()
	{
		SkyTexture = "RenderFX/SkyGradient.png";
		MoonTexture = "RenderFX/Moon.png";

		SkyVOff = 0;
		SunPitch = 45;
	}

	DF_PROPERTY_BEGIN(EvKeyFrame)
		DF_PROPERTY(EvKeyFrame, Time, "Sky", "Time", PT_Float)
		DF_PROPERTY(EvKeyFrame, StarfieldLum, "Sky", "StarLum", PT_Float)
		DF_PROPERTY(EvKeyFrame, SkyLum, "Sky", "SkyLum", PT_Float)
		DF_PROPERTY(EvKeyFrame, SunLum, "Sky", "SunLum", PT_Float)
		DF_PROPERTY(EvKeyFrame, SunSize, "Sky", "SunSize", PT_Float)
		DF_PROPERTY(EvKeyFrame, SunPower, "Sky", "SunPower", PT_Float)
		DF_PROPERTY_EX(EvKeyFrame, SunColor, "Sky", "SunColor", "PT_Color", PT_Float3)
		DF_PROPERTY(EvKeyFrame, MoonLum, "Sky", "MoonLum", PT_Float)
		DF_PROPERTY(EvKeyFrame, MoonSize, "Sky", "MoonSize", PT_Float)
		DF_PROPERTY(EvKeyFrame, MoonPhase, "Sky", "MoonPhase", PT_Float)
		DF_PROPERTY(EvKeyFrame, WindDir, "Sky", "WindDir", PT_Float3)

		DF_PROPERTY_EX(EvKeyFrame, LightAmbient, "Light", "Ambient", "PT_Color", PT_Float3)
		DF_PROPERTY_EX(EvKeyFrame, LightDiffuse, "Light", "Diffuse", "PT_Color", PT_Float3)
		DF_PROPERTY_EX(EvKeyFrame, LightSpecular, "Light", "Specular", "PT_Color", PT_Float3)
		DF_PROPERTY(EvKeyFrame, LightStrength, "Light", "Strength", PT_Float)

		DF_PROPERTY(EvKeyFrame, FogStart, "Fog", "Start", PT_Float)
		DF_PROPERTY(EvKeyFrame, FogEnd, "Fog", "End", PT_Float)
		DF_PROPERTY_EX(EvKeyFrame, FogColor, "Fog", "Color", "PT_Color", PT_Float3)

		DF_PROPERTY(EvKeyFrame, GrassParam.WaveSpeed, "Grass", "WaveSpeed", PT_Float)
		DF_PROPERTY(EvKeyFrame, GrassParam.WaveStrength, "Grass", "WaveStrength", PT_Float)
		DF_PROPERTY(EvKeyFrame, GrassParam.VisibleRadius, "Grass", "VisibleRadius", PT_Float)

		DF_PROPERTY(EvKeyFrame, HdrParam.Exposure, "HDR", "Exposure", PT_Float)
		DF_PROPERTY(EvKeyFrame, HdrParam.Threshold, "HDR", "Threshold", PT_Float)
		DF_PROPERTY(EvKeyFrame, HdrParam.Weights, "HDR", "Weights", PT_Float3)
		DF_PROPERTY(EvKeyFrame, HdrParam.SharpScale, "HDR", "SharpScale", PT_Float)
		DF_PROPERTY(EvKeyFrame, HdrParam.SharpFadeStart, "HDR", "SharpFadeStart", PT_Float)
		DF_PROPERTY(EvKeyFrame, HdrParam.SharpFadeEnd, "HDR", "SharpFadeEnd", PT_Float)

		DF_PROPERTY(EvKeyFrame, GodRayParam.SunLum, "GodRay", "SunLum", PT_Float)
		DF_PROPERTY(EvKeyFrame, GodRayParam.SunInner, "GodRay", "SunInner", PT_Float)
		DF_PROPERTY(EvKeyFrame, GodRayParam.SunPower, "GodRay", "SunPower", PT_Float)
		DF_PROPERTY(EvKeyFrame, GodRayParam.SunSize, "GodRay", "SunSize", PT_Float)
		DF_PROPERTY(EvKeyFrame, GodRayParam.UVStep, "GodRay", "UVStep", PT_Float)
		DF_PROPERTY(EvKeyFrame, GodRayParam.Weight, "GodRay", "Weight", PT_Float)

		DF_PROPERTY(EvKeyFrame, CloudParam.Height, "Cloud", "Height", PT_Float)
		DF_PROPERTY(EvKeyFrame, CloudParam.Curved, "Cloud", "Curved", PT_Float)
		DF_PROPERTY(EvKeyFrame, CloudParam.Mass, "Cloud", "Mass", PT_Float)
		DF_PROPERTY(EvKeyFrame, CloudParam.Weights, "Cloud", "Weights", PT_Float4)
		DF_PROPERTY(EvKeyFrame, CloudParam.UVScales, "Cloud", "UVScales", PT_Float4)
		DF_PROPERTY(EvKeyFrame, CloudParam.UVScrolls, "Cloud", "UVScrolls", PT_Float4)
		DF_PROPERTY(EvKeyFrame, CloudParam.Alpha, "Cloud", "Alpha", PT_Float)
		DF_PROPERTY(EvKeyFrame, CloudParam.AlphaStrength, "Cloud", "AlphaStrength", PT_Float)
		DF_PROPERTY(EvKeyFrame, CloudParam.LightStrength, "Cloud", "LightStrength", PT_Float)
		DF_PROPERTY(EvKeyFrame, CloudParam.FadeStart, "Cloud", "FadeStart", PT_Float)
		DF_PROPERTY(EvKeyFrame, CloudParam.FadeStrength, "Cloud", "FadeStrength", PT_Float)
		DF_PROPERTY_EX(EvKeyFrame, CloudParam.Ambient, "Cloud", "Ambient", "PT_Color", PT_Float3)
		DF_PROPERTY_EX(EvKeyFrame, CloudParam.Diffuse, "Cloud", "Diffuse", "PT_Color", PT_Float3)
		DF_PROPERTY(EvKeyFrame, CloudParam.AmbientScale, "Cloud", "AmbientScale", PT_Float)
		DF_PROPERTY(EvKeyFrame, CloudParam.DiffuseScale, "Cloud", "DiffuseScale", PT_Float)

		DF_PROPERTY_EX(EvKeyFrame, OceanParam.DeepColor, "Water", "DeepColor", "PT_Color", PT_Float3)
		DF_PROPERTY_EX(EvKeyFrame, OceanParam.ReflColor, "Water", "ReflColor", "PT_Color", PT_Float3)
		DF_PROPERTY(EvKeyFrame, OceanParam.FogStart, "Water", "fogStart", PT_Float)
		DF_PROPERTY(EvKeyFrame, OceanParam.FogEnd, "Water", "FogEnd", PT_Float)
		DF_PROPERTY_EX(EvKeyFrame, OceanParam.FogColor, "Water", "FogColor", "PT_Color", PT_Float3)
		DF_PROPERTY(EvKeyFrame, OceanParam.RayLum, "Water", "RayLum", PT_Float)
		DF_PROPERTY(EvKeyFrame, OceanParam.RayExp0, "Water", "RayExp0", PT_Float)
		DF_PROPERTY(EvKeyFrame, OceanParam.RayExp1, "Water", "RayExp1", PT_Float)

	DF_PROPERTY_END();

	EvKeyFrame::EvKeyFrame()
	{
		Time = 8;

		StarfieldLum = 0;
		SkyLum = 1;
		SunSize = 40 * UNIT_METRES;
		SunLum = 1;
		SunPower = 3;
		SunColor = Float3(1, 0.95f, 0.76f);

		MoonLum = 1;
		MoonSize = 20 * UNIT_METRES;
		MoonPhase = 1.4f;

		WindDir = Float3(1, 0, 0);

		LightAmbient = Float3(0.5f, 0.5f, 0.5f);
		LightDiffuse = Float3(1.0f, 1.0f, 1.0f);
		LightSpecular = Float3(0.0f, 0.0f, 0.0f);
		LightStrength = 1;

		FogStart = 50 * UNIT_METRES;
		FogEnd = 200 * UNIT_METRES;
		FogColor = Float3(1.0f, 1.0f, 1.0f);
	}

	void EvKeyFrame::Lerp(EvKeyFrame & o, const EvKeyFrame & k0, const EvKeyFrame & k1, float t)
	{
		int size = k0.GetPropertySize();
		for (int i = 0; i < size; ++i)
		{
			const Property * p = k0.GetProperty(i);

			const void * data0 = k0.GetPropertyData(p);
			const void * data1 = k1.GetPropertyData(p);

			d_assert (data0 != NULL && data1 != NULL);

			switch (p->type)
			{
			case PT_Float:
				{
					float v0 = p->AsFloat(data0);
					float v1 = p->AsFloat(data1);
					float v = v0 + (v1 - v0) * t;

					o.SetPropertyData(p, &v);
				}
				break;

			case PT_Float2:
				{
					Float2 v0 = p->AsFloat2(data0);
					Float2 v1 = p->AsFloat2(data1);
					Float2 v = v0 + (v1 - v0) * t;

					o.SetPropertyData(p, &v);
				}
				break;

			case PT_Float3:
				{
					Float3 v0 = p->AsFloat3(data0);
					Float3 v1 = p->AsFloat3(data1);
					Float3 v = v0 + (v1 - v0) * t;

					o.SetPropertyData(p, &v);
				}
				break;

			case PT_Float4:
				{
					Float4 v0 = p->AsFloat4(data0);
					Float4 v1 = p->AsFloat4(data1);
					Float4 v = v0 + (v1 - v0) * t;

					o.SetPropertyData(p, &v);
				}
				break;
			}
		}
	}

}