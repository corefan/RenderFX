/*
*	Snow
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

	class RFX_ENTRY Snow
	{
		DECLARE_ALLOC();

		typedef Array<Particle> ParticlePool;
		typedef Stack<Particle*> ParticleStack; 
		typedef Array<Particle*> ParticleArray; 

	public:
		int Quota;
		int Rate;
		Float3 Extend;
		float MinSize;
		float MaxSize;
		float MinSpeed;
		float MaxSpeed;
		float MinForce;
		float MaxForce;
		float MinLife;
		float MaxLife;
		bool Enabled;

	public:
		Snow();
		~Snow();

		void Reset();

		void Update(float elapsedTime);

		void Render();

	private:
		Particle * _quest_particle();
		void _free_particle(Particle * p);

		void _initParticle(Particle * p);
		void _affectParticle(Particle * p, float elapsedTime);
		void _updateBuffer();

	protected:
		RenderOp mRenderOp;
		ShaderFX * mTech;

		ParticlePool mParticlePool;
		ParticleStack mParticleFreeStack;
		ParticleArray mParticles;

		float mInternalTime;
		float mLastEmitTime;

		bool mEnable;
	};

}
