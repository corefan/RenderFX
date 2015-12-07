/*
*	ProjectionGrid
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "MRenderSystem.h"
#include "MPerlin.h"

namespace Rad {

    class ProjectedGrid
    {
		struct Vertex {
			Float3 Position;
			Float3 Normal;
		};

    public:
		ProjectedGrid();
        ~ProjectedGrid();

		void 
			SetElevation(float height);
        void 
			Update(float elapsedTime);

        RenderOp * 
			GetRenderOp() { return &mRenderOp; }
		const Plane & 
			GetUpperPlane() { return mUpperBound; }
		const Plane & 
			GetLowerPlane() { return mLowerBound; }

    protected:
        bool 
			_getMinMax(Mat4 & range);
        void 
			_setDisplacementAmplitude(float ampl);
        Float4
			_calculeWorldPosition(const Float2 &uv, const Mat4& m);

        void 
			_renderGeometry(const Mat4& m, const Float3& WorldPos);

        void 
			_updateNormals();

    protected:
		Perlin * mNoise;
		int mComplexityU;
		int mComplexityV;
		float mStrength;
		float mElevation;

		Float4 t_corners0,t_corners1,t_corners2,t_corners3;
	    Plane mPlane, mUpperBound, mLowerBound;


		Vertex *mVertexBuffer;
        RenderOp mRenderOp;
    };
}