#include "MRain.h"
#include "MWorld.h"
#include "MRenderFX.h"

namespace Rad {

	struct RainVertex {
		Float3 position;
		Float2 uv;
		float opacity;
	};

	Rain::Rain()
	{
		Quota = 1000;
		Rate = 200;
		Extend = Float3(4, 1, 4) * UNIT_METRES;
		Speed = 4 * UNIT_METRES;
		Life = 3;
		Enabled = false;

		mTech = NULL;

		mInternalTime = mLastEmitTime = 0;
	}

	Rain::~Rain()
	{
	}

	void Rain::Reset()
	{
		d_assert (Quota > 0 && Rate > 0);

		mTech = ShaderFXManager::Instance()->Load("Rain", "RenderFX/MRain.mfx");
		
		mRenderOp.vertexDeclarations[0].Clear();
		mRenderOp.vertexDeclarations[0].AddElement(eVertexSemantic::POSITION, eVertexType::FLOAT3);
		mRenderOp.vertexDeclarations[0].AddElement(eVertexSemantic::TEXCOORD0, eVertexType::FLOAT3);

		mRenderOp.vertexBuffers[0] = HWBufferManager::Instance()->NewVertexBuffer(6 * sizeof(float), Quota * 4, eUsage::DYNAMIC_MANAGED);

		mRenderOp.indexBuffer = HWBufferManager::Instance()->NewIndexBuffer(Quota * 6);
		short * idx = (short *)mRenderOp.indexBuffer->Lock(eLockFlag::WRITE);
		for (int i = 0; i < Quota; ++i)
		{
			*idx++ = i * 4 + 0;
			*idx++ = i * 4 + 1;
			*idx++ = i * 4 + 2;

			*idx++ = i * 4 + 2;
			*idx++ = i * 4 + 1;
			*idx++ = i * 4 + 3;
		}
		mRenderOp.indexBuffer->Unlock();

		mRenderOp.primCount = 0;
		mRenderOp.primType = ePrimType::TRIANGLE_LIST;

		mParticles.Clear();
		mParticleFreeStack.Clear();
		mParticlePool.Clear();

		mParticlePool.Resize(Quota);
		for (int i = 0; i < Quota; ++i)
		{
			mParticleFreeStack.Push(&mParticlePool[i]);
		}

		mInternalTime = mLastEmitTime = 0;

		for (int i = 0; i < mRandomDirection.Size(); ++i)
		{
			mRandomDirection[i] = Float3(0, -1.0f, 0);
			mRandomDirection[i].x = Math::RandRange(0.15f, 0.2f);
			mRandomDirection[i].Normalize();
		}

		mRandomUV[0] = RectF(0, 0, 0.5f, 1.0f);
		mRandomUV[1] = RectF(0.5f, 0, 1.0f, 1.0f);
	}

	void Rain::Update(float elapsedTime)
	{
		if (!Enabled)
			return ;

		if (mParticlePool.Empty())
		{
			Reset();
		}

		mCamPos = World::Instance()->MainCamera()->GetWorldPosition();
		World::Instance()->MainCamera()->GetWorldRotation().ToAxis(mCamXAxis, mCamYAxis, mCamZAxis);

		// update
		for (int i = 0; i < mParticles.Size(); ++i)
		{
			Particle * p = mParticles[i];

			p->Life -= elapsedTime;

			if (p->Life < 0)
			{
				mParticles.Erase(i--);
				_free_particle(p);
			}
			else
			{
				p->Position += p->Direction * p->Speed * elapsedTime;
			}
		}

		// emit
		mInternalTime += elapsedTime;

		int emitCount = 0;
		mInternalTime += elapsedTime;

		float time = mInternalTime - mLastEmitTime;
		float emit_time = 1.0f / Rate;

		while (time > emit_time)
		{
			mLastEmitTime = mInternalTime;
			++emitCount;
			time -= emit_time;
		}

		for (int i = 0; i < emitCount; ++i)
		{
			Particle * p = _quest_particle();

			if (!p) break;

			_initParticle(p);

			mParticles.PushBack(p);
		}

		// affect
		for (int i = 0; i < mParticles.Size(); ++i)
		{
			_affectParticle(mParticles[i], elapsedTime);
		}

		//
		_updateBuffer();
	}

	void Rain::Render()
	{
		if (mRenderOp.primCount)
		{
			RenderSystem::Instance()->SetWorldTM(Mat4::Identity);
			RenderSystem::Instance()->Render(mTech, &mRenderOp);
		}
	}

	Particle * Rain::_quest_particle()
	{
		Particle * p = NULL;

		if (Enabled && !mParticleFreeStack.Empty())
		{
			p = mParticleFreeStack.Top();
			mParticleFreeStack.Pop();
		}

		return p;
	}

	void Rain::_free_particle(Particle * p)
	{
		mParticleFreeStack.Push(p);
	}

	void Rain::_initParticle(Particle * p)
	{
		p->Color = Float4(1, 1, 1, 1);
		p->Speed = Speed;
		p->Life = Life;
		p->Size = Float3(0.8f, 20, 1);

		p->InitColor = p->Color;
		p->InitSize = p->Size;
		p->InitLife = Float2(p->Life, 1 / p->Life);

		float x = Math::RandRange(-Extend.x, Extend.x);
		float y = Math::RandRange(-Extend.y, Extend.y);
		float z = Math::RandRange(-Extend.z, Extend.z);
		p->Position = mCamPos + Float3(x, y, z) + mCamZAxis * UNIT_METRES;
		p->Position.y += 1 * UNIT_METRES;

		p->UVRect = mRandomUV[Math::RandRange(0, mRandomUV.Size() - 1)];
		p->Direction = mRandomDirection[Math::RandRange(0, mRandomDirection.Size() - 1)];
	}

	void Rain::_affectParticle(Particle * p, float elapsedTime)
	{
	}

	void Rain::_updateBuffer()
	{
		Float3 camPos = World::Instance()->MainCamera()->GetWorldPosition();

		mRenderOp.primCount = mParticles.Size() * 2;

		if (mRenderOp.primCount == 0)
			return ;

		int count = mParticles.Size();
		d_assert (count * 4 <= mRenderOp.vertexBuffers[0]->GetCount());

		RainVertex * v = (RainVertex *)mRenderOp.vertexBuffers[0]->Lock(eLockFlag::WRITE);
		for (int i = 0; i < count; ++i)
		{
			const Particle * p = mParticles[i];

			float hw = Max(0.0f, p->Size.x / 2);
			float hh = Max(0.0f, p->Size.y / 2);

			Float3 xAxis, yAxis;

			yAxis = p->Direction;
			xAxis = Float3::Cross(yAxis, mCamZAxis);

			xAxis *= hw, yAxis *= hh;

			v->position = p->Position - xAxis + yAxis;
			v->uv.x = p->UVRect.x1, v->uv.y = p->UVRect.y1;
			v->opacity = p->Color.a;
			++v;

			v->position = p->Position + xAxis + yAxis;
			v->uv.x = p->UVRect.x2, v->uv.y = p->UVRect.y1;
			v->opacity = p->Color.a;
			++v;

			v->position = p->Position - xAxis - yAxis;
			v->uv.x = p->UVRect.x1, v->uv.y = p->UVRect.y2;
			v->opacity = p->Color.a;
			++v;

			v->position = p->Position + xAxis - yAxis;
			v->uv.x = p->UVRect.x2, v->uv.y = p->UVRect.y2;
			v->opacity = p->Color.a;
			++v;
		}
		mRenderOp.vertexBuffers[0]->Unlock();
	}

}

