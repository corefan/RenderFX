#include "MStarfield.h"
#include "MAstronomy.h"
#include "MRenderFX.h"
#include <vector>
#include <algorithm>

#pragma warning (push)
#pragma warning (disable : 4244)

namespace Rad {

    Starfield::Starfield()
    {
        mTech = ShaderFXManager::Instance()->Load("Starfield.mfx", "RenderFX/MStarfield.mfx");

        mStars = NULL;
        mSize = 0;
        mIterator = 0;

        mMag0PixelSize = 16;
        mMinPixelSize = 2;
        mMaxPixelSize = 4;
        mMagnitudeScale = Math::Pow(100, 0.2f);
        mObserverLatitude = 45;
        mObserverLongitude = 0;

		Resize(2000);
	}

    Starfield::~Starfield()
    {
        safe_delete (mStars);
    }

    void Starfield::Render()
    {
        if (mSize == 0)
            return ;

		RenderContext * context = World::Instance()->GetCurrentRenderContext();

        RenderSystem * render = RenderSystem::Instance();
        Camera * cam = World::Instance()->MainCamera();

        int width = context->GetViewport().w;
		int height = context->GetViewport().h;
        float pixFactor = 1.0f / width;
        float magScale = -Math::Log(mMagnitudeScale) / 2;
        float mag0Size = mMag0PixelSize * pixFactor;
        float minSize = mMinPixelSize * pixFactor;
        float maxSize = mMaxPixelSize * pixFactor;
        float aspect = cam->GetAspect();
        float farclip = cam->GetFarClip() * 0.9f;
        Float3 pos = cam->GetPosition();
		float starLum = RenderFX::Instance()->GetEvParam()->StarfieldLum;

        FX_Uniform * uSizeParam = mTech->GetPass(0)->GetUniform("gSizeParam");
        FX_Uniform * uTransform = mTech->GetPass(0)->GetUniform("gTransform");
		FX_Uniform * uAspect = mTech->GetPass(0)->GetUniform("gAspect");
        FX_Uniform * uStarLum = mTech->GetPass(0)->GetUniform("gStarLum");

        uSizeParam->SetConst(magScale, mag0Size, minSize, maxSize);
        uTransform->SetConst(pos.x, pos.y, pos.z, farclip);
        uAspect->SetConst(aspect, 0, 0, 0);
		uStarLum->SetConst(starLum, 0, 0, 0);

		RENDER_EVENT_BEGIN("Render Starfield");

        render->Render(mTech, &mRenderOp);

		RENDER_EVENT_END();
    }

    void Starfield::Resize(int size)
    {
        if (mSize == size)
            return ;

        mSize = size;

        if (mSize == 0)
            return ;

        _random();
        _geometry();

        safe_delete (mStars);
    }

    void Starfield::_random()
    {
        safe_delete (mStars);
        mIterator = 0;

        if (mSize > BrightStarCatalogueSize)
            mSize = BrightStarCatalogueSize;

		mStars = new Star[mSize];

        if (mSize < BrightStarCatalogueSize) 
        {
            std::vector<std::pair<float, int> > vec;
            vec.reserve(BrightStarCatalogueSize);

            for (int i = 0; i < BrightStarCatalogueSize; ++i)
                vec.push_back(std::make_pair(BrightStarCatalogue[i].magn, i));

            sort(vec.begin(), vec.end());

            for (int i = 0; i < mSize; ++i)
                _add(BrightStarCatalogue[vec[i].second]);
        }
        else
        {
            for (int i = 0; i < BrightStarCatalogueSize; ++i)
                _add(BrightStarCatalogue[i]);
        }
    }

    void Starfield::_geometry()
    {
        int iVertexCount = mSize * 4;
        int iIndexCount = mSize * 6;
        int iPrimCount = iIndexCount / 3;
		int iStride = 28;

        VertexDeclaration decl;
		decl.AddElement(eVertexSemantic::POSITION, eVertexType::FLOAT4);
        decl.AddElement(eVertexSemantic::TEXCOORD0, eVertexType::FLOAT3);

		mRenderOp.vertexDeclarations[0] = decl;

        VertexBufferPtr vb = HWBufferManager::Instance()->NewVertexBuffer(iStride, iVertexCount);
        IndexBufferPtr ib = HWBufferManager::Instance()->NewIndexBuffer(iIndexCount * 2);

        float * vert = (float *)vb->Lock(eLockFlag::WRITE);
        for (int i = 0; i < mSize; ++i)
        {
            const Star & star = mStars[i];

            // Determine position at J2000
            double azm, alt;
            Astronomy::convertEquatorialToHorizontal(
                Astronomy::J2000, mObserverLatitude, mObserverLongitude,
                star.RightAscension, star.Declination, azm, alt);

            azm = Math::DegreeToRadian(azm);
            alt = Math::DegreeToRadian(alt);

            float z = -Math::Cos(azm) * Math::Cos(alt);
            float x =  Math::Sin(azm) * Math::Cos(alt);
            float y = -Math::Sin(alt);
            float m = star.Magnitude;
			float phase = Math::RandRange(0.0f, PI * 2.0f);

            *vert++ = x; *vert++ = y; *vert++ = z; *vert++ = phase;
            *vert++ = -1; *vert++ = 1; *vert++ = m;

            *vert++ = x; *vert++ = y; *vert++ = z; *vert++ = phase;
            *vert++ = 1; *vert++ = 1; *vert++ = m;

            *vert++ = x; *vert++ = y; *vert++ = z; *vert++ = phase;
            *vert++ = -1; *vert++ = -1; *vert++ = m;

            *vert++ = x; *vert++ = y; *vert++ = z; *vert++ = phase;
            *vert++ = 1; *vert++ = -1; *vert++ = m;
        }
        vb->Unlock();


        unsigned short * idx = (unsigned short *)ib->Lock(eLockFlag::WRITE);
        {
            for (unsigned short i = 0; i < (unsigned short)mSize; ++i)
            {
                unsigned short index = i * 4;

                *idx++ = index + 0;
                *idx++ = index + 1;
                *idx++ = index + 2;

                *idx++ = index + 2;
                *idx++ = index + 1;
                *idx++ = index + 3;
            }
        }
        ib->Unlock();

		mRenderOp.vertexBuffers[0] = vb;
		mRenderOp.indexBuffer = ib;

        mRenderOp.primCount = iPrimCount;
        mRenderOp.primType = ePrimType::TRIANGLE_LIST;
    }

    void Starfield::_add(const BrightStarCatalogueEntry & entry)
    {
        Star & s = mStars[mIterator++];

        s.RightAscension = 360 / 24.0f * (
            Math::Abs(entry.rasc_hour) +
            entry.rasc_min / 60.0f +
            entry.rasc_sec / 3600.0f);

        s.Declination = Math::Sign(entry.decl_deg) * (
            Math::Abs(entry.decl_deg) +
            entry.decl_min / 60.0f +
            entry.decl_sec / 3600.0f);

        s.Magnitude = entry.magn;
    }
}


#pragma warning (pop)
