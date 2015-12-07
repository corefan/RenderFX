#include "DebugInfo.h"

ImplementSingleton(DebugInfo);

DebugInfo::DebugInfo()
{
	mLayout = new MGUI::Layout(NULL);
	mLayout->SetRect(0, 0, 128, 128);

	mLayout->SetPickFlag(MGUI::ePickFlag::NONE);
	mLayout->SetAlign(MGUI::eAlign::RIGHT | MGUI::eAlign::BOTTOM);

	mLogo = new MGUI::ImageBox(NULL, mLayout);
	mLogo->SetAlign(MGUI::eAlign::RIGHT | MGUI::eAlign::TOP);
	mLogo->SetSkin("Logo.png");
	mLogo->SetRect(0, 0, 128, 128);

	mBatch = new MGUI::TextBox(NULL, mLayout);
	mBatch->SetRect(0, 0, 128, 27);

	mPrimitive = new MGUI::TextBox(NULL, mLayout);
	mPrimitive->SetRect(0, 27 + 10, 128, 27);

	mFPS = new MGUI::TextBox(NULL, mLayout);
	mFPS->SetRect(0, 27 + 10 + 27 + 10, 128, 27);

#ifdef M_PLATFORM_WIN32
	Show(true);
#else
	Show(false);
#endif
}

DebugInfo::~DebugInfo()
{
	delete mLayout;
}

void DebugInfo::Update()
{
	if (!mLayout->IsVisible())
		return ;

	int batchCount = RenderSystem::Instance()->_getBatchCount();
	int primCount = RenderSystem::Instance()->_getPrimitiveCount();
	int fps = (int)Root::Instance()->GetFPS();

	String text_batch, text_prim, text_fps;

	text_batch.Format("BC: %d", batchCount);
	text_prim.Format("TC: %d", primCount);
	text_fps.Format("FPS: %d", fps);

	mBatch->SetCaption(text_batch.c_wstr());
	mPrimitive->SetCaption(text_prim.c_wstr());
	mFPS->SetCaption(text_fps.c_wstr());
}

void DebugInfo::Show(bool _show)
{
	mLayout->SetVisible(_show);
}