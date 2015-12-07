#include "App.h"
#include "MRenderFX.h"

class DemoEX_RenderFX : public App
{
	RenderFX * mRenderFX;

public:
	DemoEX_RenderFX() {}
	virtual ~DemoEX_RenderFX() {}

	virtual void OnSetupResource()
	{
#ifdef M_PLATFORM_WIN32
		ResourceManager::Instance()->AddArchive(new MPKArchive("../Core.MPK", NULL));
		ResourceManager::Instance()->AddArchive(new FilePathArchive("../Sample"));
		ResourceManager::Instance()->AddArchive(new FilePathArchive("../Sample/Brush"));
		ResourceManager::Instance()->AddArchive(new FilePathArchive("../Sample/Mesh"));
		ResourceManager::Instance()->AddArchive(new FilePathArchive("../Sample/Scene"));
#endif

#ifdef M_PLATFORM_ANDROID
		ResourceManager::Instance()->AddArchive(new APKArchive("Core"));
		ResourceManager::Instance()->AddArchive(new APKArchive("Sample"));
#endif
	}

	virtual void OnInit()
	{
		MGUI::FontManager::Instance()->Load("Sample.font");
		MGUI::LookFeelManager::Instance()->Load("Sample.lookfeel");

		MGUI::Layout * layout = new MGUI::Layout(NULL, NULL);
		layout->SetAlign(MGUI::eAlign::STRETCH);

		World::Instance()->MainRenderContext()->SetColorClear(eClearMode::ALL, Float4(0.1f, 0.1f, 0.3f));

		mRenderFX = new RenderFX;

		World::Instance()->Load("test001/test001.scene");

		World::Instance()->MainCamera()->SetPosition(5077, 860, 3720);
		World::Instance()->MainCamera()->SetRotation(-0.038690358f, 0.59063727f, 0.028420433f, 0.80406910f);
	}

	virtual void OnUpdate()
	{
		Node * cam = World::Instance()->MainCamera();
		float frameTime = Root::Instance()->GetFrameTime();
		float camMoveStep = frameTime * UNIT_METRES * 10;
		float camRotateStep = frameTime * 0.1f;
		float camWheelStep = frameTime * UNIT_METRES;

		if (IMouse::Instance()->MouseMoved() && IMouse::Instance()->KeyPressed(InputCode::MKC_RIGHT))
		{
			Float2 pt = IMouse::Instance()->GetPositionDiff();

			if (abs(pt.y) >abs(pt.x))
			{
				cam->Pitch(pt.y * camRotateStep, eTransform::LOCAL);
			}
			else
			{
				cam->Yaw(pt.x * camRotateStep, eTransform::PARENT);
			}
		}

		if (IMouse::Instance()->MouseWheel())
		{
			cam->Translate(Float3(0, 0, camWheelStep * IMouse::Instance()->MouseWheel()), eTransform::LOCAL);
		}

		mRenderFX->Update(Root::Instance()->GetFrameTime());
	}

	virtual void OnShutdown()
	{
		World::Instance()->Unload();

		delete mRenderFX;
	}

	virtual void OnPause()
	{
	}

	virtual void OnResume()
	{
	}

	virtual void OnResize(int w, int h)
	{
		mRenderFX->Resize(w, h);
	}
};

void CreateApplication(App ** ppApp)
{
	*ppApp = new DemoEX_RenderFX;
}
