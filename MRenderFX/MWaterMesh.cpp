#include "MWaterMesh.h"

namespace Rad {

	ImplementRTTI(WaterMesh, Node);

	DF_PROPERTY_BEGIN(WaterMesh)
		DF_PROPERTY(WaterMesh, mComplexityU, "WaterMesh", "ComplexityU", PT_Int)
		DF_PROPERTY(WaterMesh, mComplexityV, "WaterMesh", "ComplexityV", PT_Int)
		DF_PROPERTY(WaterMesh, mGridSize, "WaterMesh", "GridSize", PT_Float)
	DF_PROPERTY_END()

	WaterMesh::WaterMesh(int complexityU, int complexityV, float girdSize)
		: mComplexityU(complexityU)
		, mComplexityV(complexityV)
		, mGridSize(girdSize)
	{
		PARAM_C = 0.3f ; // ripple speed
		PARAM_D = 0.4f ; // distance
		PARAM_U = 0.05f ; // viscosity
		PARAM_T = 0.13f ; // time
		
		mHeightBuffer[0] = NULL;
		mHeightBuffer[1] = NULL;
		mHeightBuffer[2] = NULL;

		_createMesh();
		mMaterial.blendMode = eBlendMode::ALPHA_BLEND;
		mMaterial.depthMode = eDepthMode::N_LESS_EQUAL;

		mNode = this;
		SetTmFlags(eTmFlags::TRANSLATE);
		SetLighting(false);

		SetWaterShader(new WaterMeshShaderStandard);
	}

	WaterMesh::~WaterMesh()
	{
		safe_delete (mHeightBuffer[0]);
		safe_delete (mHeightBuffer[1]);
		safe_delete (mHeightBuffer[2]);
	}

	void WaterMesh::OnPropertyChanged(const Property * p)
	{
		if (p->name == "mComplexityU" ||
			p->name == "mComplexityV")
			_createMesh();
		else
			Node::OnPropertyChanged(p);
	}

	void WaterMesh::Push(int u, int v, float depth, bool absolute)
	{
		d_assert (u >= 0 && u < mComplexityU && v >= 0 && v < mComplexityV);

		float * buf = mHeightBuffer[mCurrentIndex];

		if (!absolute)
			buf[v * mComplexityU + u] += depth;
		else
			buf[v * mComplexityU + u] = depth;
	}

	bool WaterMesh::PushAt(float x, float z, float depth, bool absolute)
	{
		const Float3 & worldPos = GetWorldPosition();

		x -= worldPos.x;
		z -= worldPos.z;
		x /= mGridSize;
		z /= mGridSize;

		if (x <= 0 || x >= mComplexityU-1 || z <= 0 || z >= mComplexityV-1)
			return false;

		int x0 = (int)x;
		int z0 = (int)z;

		for (int j = 0; j <= 1; ++ j)
		{
			for (int i = 0; i <= 1; ++i)
			{
				int u = x0 + i;
				int v = z0 + j;

				float dx = u - x;
				float dz = v - z;

				float k = Max(0.0f, 1 - Math::Sqrt(dx * dx + dz * dz));

				Push(u, v, k * depth, absolute);
			}
		}

		return true;
	}

	float WaterMesh::GetHeight(int u, int v)
	{
		d_assert (u >= 0 && u < mComplexityU && v >= 0 && v < mComplexityV);

		return mVertexBuffer[v * mComplexityU + u].Position.y;
	}

	bool WaterMesh::GetHeightAt(float & h, float x, float z)
	{
		const Float3 & worldPos = GetWorldPosition();

		x -= worldPos.x;
		z -= worldPos.z;
		x /= mGridSize;
		z /= mGridSize;

		if (x < 0 || x > mComplexityU-1 || z < 0 || z > mComplexityV-1)
			return false;

		int x0 = (int)x;
		int z0 = (int)z;
		int x1 = x0 + 1;
		int z1 = z0 + 1;

		x1 = Min(mComplexityU - 1, x1);
		z1 = Min(mComplexityV - 1, z1);

		float dx = x - x0;
		float dz = z - z0;

		float h00 = GetHeight(x0, z0);
		float h01 = GetHeight(x1, z0);
		float h10 = GetHeight(x0, z1);
		float h11 = GetHeight(x1, z1);

		float ha = h00 + (h01 - h00) * dx;
		float hb = h10 + (h11 - h10) * dx;

		h = ha + (hb - ha) * dz;

		return true;
	}

	void WaterMesh::_createMesh()
	{
		safe_delete (mHeightBuffer[0]);
		safe_delete (mHeightBuffer[1]);
		safe_delete (mHeightBuffer[2]);

		int iVertexCount = mComplexityU * mComplexityV;
		int iIndexCount = 6 * (mComplexityU - 1) * (mComplexityV - 1);
		int iPrimCount = iIndexCount / 3;
		int iStride = 24;

		VertexDeclaration decl;
		decl.AddElement(eVertexSemantic::POSITION, eVertexType::FLOAT3);
		decl.AddElement(eVertexSemantic::NORMAL, eVertexType::FLOAT3);

		VertexBufferPtr vb = HWBufferManager::Instance()->NewVertexBuffer(iStride, iVertexCount, eUsage::DYNAMIC_MANAGED);
		mVertexBuffer = (Vertex *)vb->Lock(eLockFlag::WRITE);
		{
			int index = 0;
			for (int j = 0; j < mComplexityV; j++)
			{
				for (int i = 0; i < mComplexityU; ++i)
				{
					float x = (float)i / (mComplexityU - 1);
					float z = (float)j / (mComplexityV - 1);

					mVertexBuffer[index].Position = Float3(x, 0, z);
					mVertexBuffer[index].Normal = Float3(0, 1, 0);

					++index;
				}
			}
		}
		vb->Unlock();

		IndexBufferPtr ib = HWBufferManager::Instance()->NewIndexBuffer(iIndexCount);
		short * indexbuffer = (short *)ib->Lock(eLockFlag::WRITE);
		{
			int i = 0;
			for(int v=0; v<mComplexityV-1; v++){
				for(int u=0; u<mComplexityU-1; u++){
					// face 1 |/
					indexbuffer[i++] = v * mComplexityU + u;
					indexbuffer[i++] = v * mComplexityU + u + 1;
					indexbuffer[i++] = (v+1) * mComplexityU + u;

					// face 2 /|
					indexbuffer[i++] = (v+1) * mComplexityU + u;
					indexbuffer[i++] = v * mComplexityU + u + 1;
					indexbuffer[i++] = (v+1) * mComplexityU + u + 1;
				}
			}
		}
		ib->Unlock();

		mRenderOp.vertexDeclarations[0] = decl;
		mRenderOp.vertexBuffers[0] = vb;
		mRenderOp.indexBuffer = ib;
		mRenderOp.primCount = iPrimCount;
		mRenderOp.primType = ePrimType::TRIANGLE_LIST;

		mHeightBuffer[0] = new float[iVertexCount];
		mHeightBuffer[1] = new float[iVertexCount];
		mHeightBuffer[2] = new float[iVertexCount];

		memset(mHeightBuffer[0], 0, sizeof(float) * iVertexCount);
		memset(mHeightBuffer[1], 0, sizeof(float) * iVertexCount);
		memset(mHeightBuffer[2], 0, sizeof(float) * iVertexCount);
	}

	void WaterMesh::_updateMesh(float frameTime)
	{
		const float ANIMATIONS_PER_SECOND = 60;

		mLastTime += frameTime;

		int x, y;
		while (mLastAnimationTime <= mLastTime)
		{
			mCurrentIndex = (mCurrentIndex + 1) % 3;
			float *buf = mHeightBuffer[mCurrentIndex];
			float *buf1 = mHeightBuffer[(mCurrentIndex+2)%3];
			float *buf2 = mHeightBuffer[(mCurrentIndex+1)%3];

			float C = PARAM_C; // ripple speed
			float D = PARAM_D; // distance
			float U = PARAM_U; // viscosity
			float T = PARAM_T; // time
			float TERM1 = ( 4.0f - 8.0f*C*C*T*T/(D*D) ) / (U*T+2) ;
			float TERM2 = ( U*T-2.0f ) / (U*T+2.0f) ;
			float TERM3 = ( 2.0f * C*C*T*T/(D*D) ) / (U*T+2) ;
			for(y=1;y<mComplexityV-1;y++)
			{
				float *row = buf + y*mComplexityU;
				float *row1 = buf1 + y*mComplexityU;
				float *row1up = buf1 + (y-1)*mComplexityU;
				float *row1down = buf1 + (y+1)*mComplexityU;
				float *row2 = buf2 + y*mComplexityU;
				for(x=1;x<mComplexityU-1;x++)
				{
					row[x] = 
						TERM1 * row1[x] + 
						TERM2 * row2[x] +
						TERM3 * ( row1[x-1] + row1[x+1] + row1up[x]+row1down[x] ) ;
				}
			}

			mLastAnimationTime += (1.0f / ANIMATIONS_PER_SECOND);
		}

		mRenderOp.vertexBuffers[0]->Lock(eLockFlag::WRITE);
		for (int i = 0; i < mComplexityU * mComplexityV; ++i)
		{
			mVertexBuffer[i].Position.y = mHeightBuffer[mCurrentIndex][i];
		}

		_updateNormals();

		mRenderOp.vertexBuffers[0]->Unlock();
	}

	void WaterMesh::_updateNormals()
	{
        int v, u;
        Float3 vec1, vec2, normal;

        Vertex * Vertices = static_cast<Vertex*>(mVertexBuffer);

        for (int i = 0; i < mComplexityU * mComplexityV; ++i)
        {
            Vertices[i].Normal = Float3::Zero;
        }

		int pr = 0, r = mComplexityU, nr = r + mComplexityU;

		for(v=1; v<(mComplexityV-1); v++)
		{
			for(u=1; u<(mComplexityU-1); u++)
			{
				vec1 = Vertices[r + u + 1].Position - Vertices[r + u - 1].Position;
				vec2 = Vertices[nr + u].Position - Vertices[pr + u].Position;

				normal = Float3::Cross(vec2, vec1);

                Vertices[r + u].Normal += normal;
            }

			r += mComplexityU;
			pr = r - mComplexityU;
			nr = r + mComplexityU;
        }

		int last_r = (mComplexityV - 1) * mComplexityU;
		for (int u = 0; u < mComplexityU; ++u)
		{
			Vertices[0 + u].Normal.y = 1;
			Vertices[last_r + u].Normal.y = 1;
		}

		r = 0;
		for (int v = 0; v < mComplexityV; ++v)
		{
			Vertices[r + 0].Normal.y = 1;
			Vertices[r + (mComplexityU - 1)].Normal.y = 1;

			r += mComplexityU;
		}

		/*for(v=1; v<(mOptions.ComplexityV-1); v++)
		{
			for(u=1; u<(mOptions.ComplexityU-1); u++)
			{
				int cr = v * mOptions.ComplexityU;
				int pr = (v - 1) * mOptions.ComplexityU;
				int nr = (v + 1) * mOptions.ComplexityU;

				const Float3 & p = Vertices[cr + u].Position;
				const Float3 & a = Vertices[cr + u - 1].Position;
				const Float3 & b = Vertices[pr + u].Position;
				const Float3 & c = Vertices[cr + u + 1].Position;
				const Float3 & d = Vertices[nr + u].Position;

				Float3 L = a - p, T = b - p, R = c - p, B = d - p;

				Float3 N = Float3::Zero;
				N += T.CrossN(L);
				N += L.CrossN(B);
				N += B.CrossN(R);
				N += R.CrossN(T);

				Vertices[cr + u].Normal = N;
			}
		}*/
	}

	void WaterMesh::Update(float elapsedTime)
	{
		Node::Update(elapsedTime);

		_updateMesh(elapsedTime);

		if (mWaterShader != NULL)
		{
			mWaterShader->Update(this, elapsedTime);
		}
	}

	void WaterMesh::AddRenderQueue(RenderQueue * rq)
	{
		SetRenderCallBack(eRenderCallBack::SHADER, mWaterShader.c_ptr());

		rq->AddRenderObject(this);
	}

	void WaterMesh::SetComplexity(int u, int v)
	{
		if (u > 1 && v > 1)
		{
			mComplexityU = u;
			mComplexityV = v;

			_createMesh();
		}
	}

	void WaterMesh::SetGridSize(float gridSize)
	{
		mGridSize = gridSize;
	}
	
	void WaterMesh::SetShaderClass(const FixedString32 & name)
	{
		mShaderClass = name;

		mWaterShader = Root::NEW_OBJECT_T<WaterMeshShader>(name.c_str());
		d_assert (mWaterShader != NULL);
	}

	void WaterMesh::SetWaterShader(WaterMeshShaderPtr shader)
	{
		mWaterShader = shader;

		mShaderClass = shader->GetRTTI()->Name();
	}

}