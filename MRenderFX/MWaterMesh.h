/*
*	EvParam
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "MRenderFXEntry.h"
#include "MWaterMeshShader.h"

namespace Rad {

	class RFX_ENTRY WaterMesh : public Node, public RenderObject
	{
		DECLARE_RTTI();
		DECLARE_PROPERTY(Node);

	public:
		struct Vertex {
			Float3 Position;
			Float3 Normal;
		};

	protected:
		int mComplexityU;
		int mComplexityV;
		float mGridSize;
		
		FixedString32 mShaderClass;

	public:
		WaterMesh(int complexityU = 65, int complexityV = 65, float gridSize = 0.25f * UNIT_METRES);
		virtual ~WaterMesh();

		virtual void
			OnPropertyChanged(const Property * p);

		void
			Push(int u, int v, float depth, bool absolute = false);
		bool
			PushAt(float x, float z, float depth, bool absolute = false);
		
		float
			GetHeight(int u, int v);
		bool
			GetHeightAt(float & h, float x, float z);

		virtual void
			Update(float elapsedTime);
		virtual void 
			AddRenderQueue(RenderQueue * rq);

		void
			SetComplexity(int u, int v);
		int
			GetComplexityU() { return mComplexityU; }
		int
			GetComplexityV() { return mComplexityV; }
		
		void
			SetGridSize(float gridSize);
		float
			GetGridSize() { return mGridSize; }

		float
			GetWidth() { return mGridSize * (mComplexityU - 1); }
		float
			GetLength() { return mGridSize * (mComplexityV - 1); }

		void
			SetShaderClass(const FixedString32 & name);
		const FixedString32 &
			GetShaderClass() { return mShaderClass; }
		void
			SetWaterShader(WaterMeshShaderPtr shader);
		WaterMeshShaderPtr
			GetWaterShader() { return mWaterShader; }

	protected:
		void
			_createMesh();
		void 
			_updateMesh(float frameTime);
		void 
			_updateNormals();

	protected:
		WaterMeshShaderPtr mWaterShader;

		float * mHeightBuffer[3];
		int mCurrentIndex;

		Vertex * mVertexBuffer;

		float PARAM_C ; // ripple speed 
		float PARAM_D ; // distance
		float PARAM_U ; // viscosity
		float PARAM_T ; // time
		
		float mLastTime;
		float mLastAnimationTime;
	};

}