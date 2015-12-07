#include "MProjectionGrid.h"
#include "MRenderFX.h"
#include "MWorld.h"

namespace Rad {

    #define _def_MaxFarClipDistance 99999

    ProjectedGrid::ProjectedGrid()
        : mVertexBuffer(0)
	{
		mNoise = new Perlin();

		mComplexityU = 64;
		mComplexityV = 128;
		mStrength = 0.5f * UNIT_METRES;

		SetElevation(3 * UNIT_METRES);

		int iVertexCount = mComplexityU * mComplexityV;
		int iIndexCount = 6 * (mComplexityU - 1) * (mComplexityV - 1);
		int iPrimCount = iIndexCount / 3;
		int iStride = 24;

		VertexDeclaration decl;
		decl.AddElement(eVertexSemantic::POSITION, eVertexType::FLOAT3);
		decl.AddElement(eVertexSemantic::NORMAL, eVertexType::FLOAT3);

		VertexBufferPtr vb = HWBufferManager::Instance()->NewVertexBuffer(iStride, iVertexCount, eUsage::DYNAMIC_MANAGED);
		mVertexBuffer = (Vertex *)vb->Lock(eLockFlag::WRITE);
		for (int i = 0; i < mComplexityU * mComplexityV; i++)
		{
			mVertexBuffer[i].Normal = Float3(0, 1, 0);
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
	}

	ProjectedGrid::~ProjectedGrid()
	{
		delete mNoise;
	}

	void ProjectedGrid::SetElevation(float height)
	{
		mElevation = height;

		mPlane = Plane(Float3(0, 1, 0),  Float3(0, mElevation, 0));
	}

    void ProjectedGrid::Update(float elapsedTime)
    {
        /*if (IKeyboard::Instance()->KeyPressed(KC_1))
            mRender.rState.fillMode = FILL_FRAME;
        else if (IKeyboard::Instance()->KeyPressed(KC_2))
            mRender.rState.fillMode = FILL_SOLID;*/
        mNoise->update(elapsedTime);

        Camera * cam = World::Instance()->MainCamera();
        Float3 RenderCameraPos = cam->GetPosition();
        float RenderingFarClipDistance = World::Instance()->MainCamera()->GetFarClip();

        Mat4 matRange;
        if (_getMinMax(matRange))
        {
            _renderGeometry(matRange, RenderCameraPos);

			mRenderOp.vertexBuffers[0]->Lock(eLockFlag::WRITE);
			mRenderOp.vertexBuffers[0]->Unlock();
        }
    }

    bool ProjectedGrid::_getMinMax(Mat4 & range)
    {
        _setDisplacementAmplitude(mStrength);

		Camera * cam = World::Instance()->MainCamera();
        float x_min,y_min,x_max,y_max;
	    Float3 frustum[8],proj_points[24];
	    int n_points=0;
        int cube[] = {
            0,1, 0,2, 2,3, 1,3,
            0,4, 2,6, 3,7, 1,5,
            4,6, 4,5, 5,7, 6,7
       };	

	    // transform frustum points to world space (should be done to the rendering_camera because it's the interesting one)
        Mat4 matInvVP = cam->GetViewProjTM();
		matInvVP.Inverse();

		frustum[0] = Float3(-1,-1, 0) * matInvVP;
		frustum[1] = Float3(+1,-1, 0) * matInvVP;
		frustum[2] = Float3(-1,+1, 0) * matInvVP;
		frustum[3] = Float3(+1,+1, 0) * matInvVP;
		frustum[4] = Float3(-1,-1,+1) * matInvVP;
		frustum[5] = Float3(+1,-1,+1) * matInvVP;
		frustum[6] = Float3(-1,+1,+1) * matInvVP;
		frustum[7] = Float3(+1,+1,+1) * matInvVP;

	    // check intersections with upper_bound and lower_bound	
	    for(int i = 0; i < 12; i++)
        {
		    int src=cube[i * 2], dst=cube[i * 2 + 1];

            Ray ray(frustum[src], frustum[dst] - frustum[src]);
            float dist = frustum[src].Distance(frustum[dst]);

			ray.dir.Normalize();

			float r_dist;
            if (ray.Intersect(&r_dist, mUpperBound) && r_dist < (dist + DEFAULT_EPSILON))
            {			
			    proj_points[n_points++] = frustum[src] + r_dist * ray.dir;	
		    }

            if (ray.Intersect(&r_dist, mLowerBound) && r_dist < (dist + DEFAULT_EPSILON))
            {			
			    proj_points[n_points++] = frustum[src] + r_dist * ray.dir;	
		    }
	    }

	    // check if any of the frustums points lie between the upper_bound and lower_bound planes
		for(int i = 0; i < 8; i++)
		{
			float d0 = mUpperBound.Distance(frustum[i]);
			float d1 = mLowerBound.Distance(frustum[i]);

			if ((d0 / d1) < 0)
			{			
				proj_points[n_points++] = frustum[i];
			}		
		}

		for(int i=0; i<n_points; i++)
		{
			// project the point onto the surface plane
			proj_points[i].y -= mPlane.Distance(proj_points[i]);	
		}

		for(int i=0; i<n_points; i++)
		{
			proj_points[i].Transform(cam->GetViewProjTM());
		}

	    // get max/min x & y-values to determine how big the "projection window" must be
	    if (n_points > 0){
		    x_min = proj_points[0].x;
		    x_max = proj_points[0].x;
		    y_min = proj_points[0].y;
		    y_max = proj_points[0].y;
		    for(int i=1; i<n_points; i++){
			    if (proj_points[i].x > x_max) x_max = proj_points[i].x;
			    if (proj_points[i].x < x_min) x_min = proj_points[i].x;
			    if (proj_points[i].y > y_max) y_max = proj_points[i].y;
			    if (proj_points[i].y < y_min) y_min = proj_points[i].y;
		    }		
    		
		    // build the packing matrix that spreads the grid across the "projection window"
		    Mat4 pack(x_max-x_min,	0,				0,		0,
                      0,			y_max-y_min,	0,		0,
                      0,			0,				1,		0,	
                      x_min,		y_min,			0,		1);

		    range = pack * matInvVP;

		    return true;
	    }

	    return false;
    }

    void ProjectedGrid::_setDisplacementAmplitude(float ampl)
    {
        mUpperBound = Plane(Float3(0, 1, 0), Float3(0, mElevation, 0) + ampl * mPlane.normal);
        mLowerBound = Plane(Float3(0, 1, 0), Float3(0, mElevation, 0) - ampl * mPlane.normal);
    }

    Float4 ProjectedGrid::_calculeWorldPosition(const Float2 & uv, const Mat4& m)
    {
        Mat4 viewTm = World::Instance()->MainCamera()->GetViewTM();

        Float3 origin(uv.x,uv.y,0);
        Float3 direction(uv.x,uv.y,1);

        origin = origin * m;
        direction = direction * m;

		direction = direction - origin;
		direction.Normalize();

		Ray _ray(origin,direction);
		float _len = 0;
		if (!_ray.Intersect(&_len, mPlane))
			_len = World::Instance()->MainCamera()->GetFarClip();

        Float3 worldPos = _ray.orig + _ray.dir*_len;
        Float4 _tempVec = Float4(worldPos) * viewTm;
        float _temp = -_tempVec.z;
        Float4 retPos(worldPos, 1);

        retPos /= _temp;

        return retPos;
    }

    void ProjectedGrid::_renderGeometry(const Mat4& m, const Float3& WorldPos)
    {
        Float4 t_corners[4];
        t_corners[0] = _calculeWorldPosition(Float2(0, 0), m);
        t_corners[1] = _calculeWorldPosition(Float2(1, 0), m);
        t_corners[2] = _calculeWorldPosition(Float2(0, 1), m);
        t_corners[3] = _calculeWorldPosition(Float2(1, 1), m);

        float du  = 1.0f/(mComplexityU-1),
            dv  = 1.0f/(mComplexityV-1),
            u,v = 0.0f,
            // _1_u = (1.0f-u)
            _1_u, _1_v = 1.0f,
            divide, noise;

        float x, y, z, w;

        int i = 0, iv, iu;

        Vertex * Vertices = static_cast<Vertex*>(mVertexBuffer);

        for(iv=0; iv<mComplexityV; iv++)
        {
            u = 0.0f;	
            _1_u = 1.0f;
            for(iu=0; iu<mComplexityU; iu++)
            {				
                x = _1_v*(_1_u*t_corners[0].x + u*t_corners[1].x) + v*(_1_u*t_corners[2].x + u*t_corners[3].x);				
                z = _1_v*(_1_u*t_corners[0].z + u*t_corners[1].z) + v*(_1_u*t_corners[2].z + u*t_corners[3].z);
                w = _1_v*(_1_u*t_corners[0].w + u*t_corners[1].w) + v*(_1_u*t_corners[2].w + u*t_corners[3].w);				

                divide = 1 / w;				
                x *= divide;
                z *= divide;
                noise = mNoise->getValue(x * 0.5f, z * 0.5f);
				y = -mPlane.d + noise * mStrength * 1.2f;

				//d_assert (fabs(x) < 5000 * UNIT_METRES && fabs(z) < 5000 * UNIT_METRES);

				//Vertices[i].Position = Float3(x, -mPlane.d, z);
				Vertices[i++].Position = Float3(x, y, z);

                u += du;
                _1_u = 1.0f-u;
            }
            v += dv;
            _1_v = 1.0f-v;
        }

		_updateNormals();
    }

    void ProjectedGrid::_updateNormals()
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
}