/*
*	Sun
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "MRenderFXEntry.h"
#include "MRenderObject.h"

namespace Rad {

	class RFX_ENTRY Sun
	{
		DECLARE_ALLOC();

	public:
		Sun();
		~Sun();

		void Render();

	protected:
		RenderOp mRenderOp;
		ShaderFX * mTech;
	};

}
