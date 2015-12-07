/*
*	Rain
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "MRenderFXEntry.h"
#include "MRenderObject.h"
#include "MParticleSystem.h"

namespace Rad {

	class RFX_ENTRY Rain
	{
		DECLARE_ALLOC();

		typedef Array<Particle> ParticlePool;
		typedef Stack<Particle*> ParticleStack; 
		typedef Array<Particle*> ParticleArray; 

	public:
		int Quota;
		int Rate;
		Float3 Extend;
		float Speed;
		float Life;
		bool Enabled;

	public:
		Rain();
		~Rain();

		void 
			Reset();
		void 
			Update(float elapsedTime);
		void 
			Render();

	private:
		Particle * 
			_quest_particle();
		void 
			_free_particle(Particle * p);

		void 
			_initParticle(Particle * p);
		void 
			_affectParticle(Particle * p, float elapsedTime);
		void 
			_updateBuffer();

	protected:
		RenderOp mRenderOp;
		ShaderFX * mTech;

		ParticlePool mParticlePool;
		ParticleStack mParticleFreeStack;
		ParticleArray mParticles;

		float mInternalTime;
		float mLastEmitTime;

		Field<RectF, 2> mRandomUV;
		Field<Float3, 4> mRandomDirection;

		Float3 mCamPos;
		Float3 mCamXAxis, mCamYAxis, mCamZAxis;
	};

}
