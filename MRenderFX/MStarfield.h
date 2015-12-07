#pragma once

#include "MRenderFXEntry.h"
#include "MRenderSystem.h"
#include "MBrightStarCatalogue.h"

namespace Rad {

    class RFX_ENTRY Starfield
    {
        struct Star {
            float RightAscension;
            float Declination;
            float Magnitude;
        };

    public:
        Starfield();
        ~Starfield();

        void 
			Render();
        void 
			Resize(int size);

    protected:
        void 
			_random();
        void 
			_geometry();
        void 
			_add(const BrightStarCatalogueEntry & entry);

    protected:
        float mMinPixelSize, mMaxPixelSize, mMag0PixelSize;
        float mMagnitudeScale;
        float mObserverLatitude, mObserverLongitude;

        int mIterator;
        Star * mStars;
        int mSize;

        RenderOp mRenderOp;
        ShaderFX * mTech;
    };

}

