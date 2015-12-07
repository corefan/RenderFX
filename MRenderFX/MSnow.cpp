#include "MSnow.h"
#include "MRenderFX.h"
#include "MShaderFXManager.h"
#include "MHWBufferManager.h"
#include "MRenderSystem.h"
#include "MWorld.h"

namespace Rad {

	struct SnowVertex {
		Float3 position;
		Float2 uv;
		float opacity;
	};

	Snow::Snow()
	{
		Quota = 500;
		Rate = 100;
		Extend = Float3(500, 500, 500);
		MinSize = 4;
		MaxSize = 8;
		MinSpeed = 0.4f * UNIT_METRES;
		MaxSpeed = 0.6f * UNIT_METRES;
		MinForce = 0.1f * UNIT_METRES;
		MaxForce = 0.2f * UNIT_METRES;
		MinLife = 3;
		MaxLife = 5;
		Enabled = false;

		mTech = NULL;

		mInternalTime = mLastEmitTime = 0;
	}

	Snow::~Snow()
	{
	}

	void Snow::Reset()
	{
		d_assert (Quota > 0 && Rate > 0);

		mTech = ShaderFXManager::Instance()->Load("Snow", "RenderFX/MSnow.mfx");
		
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
	}

	void Snow::Update(float elapsedTime)
	{
		if (!Enabled)
			return ;

		if (mParticlePool.Empty())
		{
			Reset();
		}

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

	void Snow::Render()
	{
		if (mRenderOp.primCount)
		{
			RenderSystem::Instance()->SetWorldTM(Mat4::Identity);
			RenderSystem::Instance()->Render(mTech, &mRenderOp);
		}
	}

	Particle * Snow::_quest_particle()
	{
		Particle * p = NULL;

		if (Enabled && !mParticleFreeStack.Empty())
		{
			p = mParticleFreeStack.Top();
			mParticleFreeStack.Pop();
		}

		return p;
	}

	void Snow::_free_particle(Particle * p)
	{
		mParticleFreeStack.Push(p);
	}

	void Snow::_initParticle(Particle * p)
	{
		p->Color = Float4(1, 1, 1, 1);
		p->Direction = Float3(0, -1, 0);
		p->Speed = Math::RandRange(MinSpeed, MaxSpeed);
		p->UVRect = RectF(0, 0, 1, 1);
		p->Life = Math::RandRange(MinLife, MaxLife);

		float d = Math::RandRange(MinSize, MaxSize);
		p->Size = Float3(d, d, d);

		Float3 v = World::Instance()->MainCamera()->GetWorldPosition();
		float x = Math::RandRange(-Extend.x, Extend.x);
		float y = Math::RandRange(-Extend.y, Extend.y);
		float z = Math::RandRange(-Extend.z, Extend.z);
		p->Position = v + Float3(x, y, z);

		p->InitColor = p->Color;
		p->InitSize = p->Size;
		p->InitLife = Float2(p->Life, 1 / p->Life);
	}

	void Snow::_affectParticle(Particle * p, float elapsedTime)
	{
		Float3 windDir = RenderFX::Instance()->GetEvParam()->WindDir;

		Float3 vForce = windDir * Math::RandRange(MinForce, MaxForce);

		Float3 Velocity = p->Direction * p->Speed;

		Velocity += vForce * elapsedTime;

		p->Direction = Velocity;
		p->Speed = p->Direction.Normalize();
	}

	void Snow::_updateBuffer()
	{
		Float3 camPos = World::Instance()->MainCamera()->GetWorldPosition();

		Float3 camXAxis, camYAxis, camZAxis;
		World::Instance()->MainCamera()->GetWorldRotation().ToAxis(camXAxis, camYAxis, camZAxis);

		mRenderOp.primCount = mParticles.Size() * 2;

		if (mRenderOp.primCount == 0)
			return ;

		int count = mParticles.Size();
		d_assert (count * 4 <= mRenderOp.vertexBuffers[0]->GetCount());

		SnowVertex * v = (SnowVertex *)mRenderOp.vertexBuffers[0]->Lock(eLockFlag::WRITE);
		for (int i = 0; i < count; ++i)
		{
			const Particle * p = mParticles[i];

			float hw = Max(0.0f, p->Size.x / 2);
			float hh = Max(0.0f, p->Size.y / 2);

			Float3 xAxis = camXAxis, yAxis = camYAxis;

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

