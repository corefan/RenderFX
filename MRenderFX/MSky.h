/*
*	Sky
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "MRenderFXEntry.h"
#include "MRenderObject.h"

namespace Rad {

	class RFX_ENTRY Sky
	{
		DECLARE_ALLOC();

	public:
		Sky();
		~Sky();

		void 
			Render();

	protected:
		RenderOp mRenderOp;
		ShaderFX * mTech;
		TexturePtr mTexture;
	};

	// atmosphere sky
	//
	class RFX_ENTRY Sky2
    {
        DECLARE_ALLOC();

		struct Options 
		{
			/// Inner atmosphere radius
			float InnerRadius;
			/// Outer atmosphere radius
			float OuterRadius;
			/// Height position, in [0, 1] range, 0=InnerRadius, 1=OuterRadius
			float HeightPosition;

			/// Rayleigh multiplier
			float RayleighMultiplier;
			/// Mie multiplier
			float MieMultiplier;
			/// Sun intensity
			float SunIntensity;

			/// WaveLength for RGB channels
			Float3 WaveLength;

			/// Phase function
			float G;

			/// Exposure coeficient
			float Exposure;

			/** Default constructor
			 */
			Options()
				: InnerRadius(9.77501f)
				, OuterRadius(10.2963f)
				, HeightPosition(0.01f)
				, RayleighMultiplier(0.0022f)
				, MieMultiplier(0.000675f)
				, SunIntensity(30)
				, WaveLength(0.57f, 0.54f, 0.44f)
				, G(-0.991f)
				, Exposure(1.5f)
			{
			}
		};

    public:
        Sky2();
        ~Sky2();

		void 
			Render();

    protected:
        void 
			_init();
		void 
			_update();

    protected:
		Options mOptions;
		ShaderFX * mTech;
        RenderOp mRenderOp;
    };
}
