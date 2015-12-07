#include "RenderFXModule.h"
#include "MainFrame.h"
#include "Editor.h"

ImplementSingleton(RenderFXModule);

RenderFXModule::RenderFXModule()
	: EditorModule("RenderFX")
{
	mRenderFX = new RenderFX;

	mLayout = MGUI::Layout::Load("RenderFX.layout", NULL);
	mLayout->SetVisible(false);

	MGUI::Button * bnCreateOcean = mLayout->GetChildT<MGUI::Button>("CreateOcean");
	bnCreateOcean->E_Click += new cListener1<RenderFXModule, const MGUI::ClickEvent *>(this, &RenderFXModule::OnCreateOcean);

	MGUI::Button * bnSave = mLayout->GetChildT<MGUI::Button>("Save");
	bnSave->E_Click += new cListener1<RenderFXModule, const MGUI::ClickEvent *>(this, &RenderFXModule::OnSave);

	MGUI::Widget * widget = mLayout->GetChild("KeyFrames");
	d_assert(widget);

	MGUI::ComboBox * cbKeyFrame = new MGUI::ComboBox(AllLookFeel::Instance()->GetComboBox(), widget->GetChildT<MGUI::Widget>("KeyFrame"));
	cbKeyFrame->SetAlign(MGUI::eAlign::STRETCH);
	for (int i = 0; i < eEvKeyType::MAX; ++i)
	{
		FixedString32 type = eEvKeyType::Instance()->GetEnum(i)->name;
		cbKeyFrame->Append(type.c_wstr());
	}
	cbKeyFrame->Append(WS("Global"));
	cbKeyFrame->SetSelectIndex(1);
	cbKeyFrame->E_SelectChanged += new cListener2<RenderFXModule, const MGUI::Event *, int>(this, &RenderFXModule::OnKeyFrameChanged);

	mPropertyPanel = new MGUI::Layout(NULL, NULL);
	mPropertyPanel->SetVisible(false);

	MGUI::Panel * panel = new MGUI::Panel(AllLookFeel::Instance()->GetPanel(), mPropertyPanel);
	panel->SetAlign(MGUI::eAlign::STRETCH);
	panel->SetVScrollBar(AllLookFeel::Instance()->GetVScrollBar());

	mPropertyGrid = new PropertyGrid(panel->GetClientWidget());
	mPropertyGrid->SetAlign(MGUI::eAlign::H_STRETCH);

	mTestLayout = new MGUI::Layout(NULL, NULL);
	mTestLayout->SetAlign(MGUI::eAlign::STRETCH);
	mTestLayout->SetPickFlag(MGUI::ePickFlag::NONE);

	mImageBox = new MGUI::ImageBox(NULL, mTestLayout);
	mImageBox->SetAlign(MGUI::eAlign::CENTER);

	Editor::Instance()->E_SceneLoaded += new cListener0<RenderFXModule>(this, &RenderFXModule::OnSceneAfterLoad);
	Editor::Instance()->E_Update += new cListener0<RenderFXModule>(this, &RenderFXModule::OnUpdate);
	Editor::Instance()->E_Resize += new cListener2<RenderFXModule, int, int>(this, &RenderFXModule::OnResize);
}

RenderFXModule::~RenderFXModule()
{
	delete mPropertyGrid;
	delete mPropertyPanel;
	delete mLayout;
	delete mRenderFX;
}

void RenderFXModule::Layout()
{
	MGUI::Rect rect = MGUI::Engine::Instance()->GetRect();
	float w = mLayout->GetRect().w;

	mLayout->SetRect(0, D_MAINMENU_H, w, rect.h - D_MAINMENU_H - D_MAINSTATUSBAR_H);
	mPropertyPanel->SetRect(w + 1, D_MAINMENU_H, 256, rect.h - D_MAINMENU_H - D_MAINSTATUSBAR_H);
}

void RenderFXModule::Show()
{
	mLayout->SetVisible(true);
	mPropertyPanel->SetVisible(true);
	mTestLayout->SetVisible(true);

	Editor::Instance()->SetSelectNode(NULL);
	Editor::Instance()->ClearFlags(FLAG_ALL);
}

void RenderFXModule::Hide()
{
	mLayout->SetVisible(false);
	mPropertyPanel->SetVisible(false);
	mTestLayout->SetVisible(false);
}

void RenderFXModule::OnSceneAfterLoad()
{
}

void RenderFXModule::OnUpdate()
{
	mRenderFX->Update(Root::Instance()->GetFrameTime());

	if (!mLayout->IsVisible())
		return ;

	if (IKeyboard::Instance()->KeyUp(InputCode::KC_F12))
	{
		if (mImageBox->GetSkin() == NULL)
		{
			mImageBox->SetSkinAlignedEx(mRenderFX->GetContext()->GetRenderTarget(0)->GetTexture());
		}
		else if (mImageBox->GetSkin() == mRenderFX->GetContext()->GetRenderTarget(0)->GetTexture())
		{
			mImageBox->SetSkinAlignedEx(mRenderFX->GetContext()->GetRenderTarget(1)->GetTexture());
		}
		else
		{
			mImageBox->SetSkinAlignedEx(NULL);
		}
	}
}

void RenderFXModule::OnResize(int w, int h)
{
	mRenderFX->Resize(w, h);
}

void RenderFXModule::OnCreateOcean(const MGUI::ClickEvent * e)
{
}

void RenderFXModule::OnKeyFrameChanged(const MGUI::Event * e, int index)
{
	if (index < eEvKeyType::MAX)
	{
		eEvKeyType type = (eEvKeyType::enum_t)(index);
		mRenderFX->SetKeyType(type);
		mPropertyGrid->Attach(mRenderFX->GetEvKeyFrame(type));
	}
	else
	{
		mPropertyGrid->Attach(mRenderFX->GetEvGlobalParam());
	}
}

void RenderFXModule::OnSave(const MGUI::ClickEvent * e)
{
	String filename = Editor::Instance()->GetSceneFilename();

	filename = FileHelper::ReplaceExternName(filename, "rfx");

	mRenderFX->Save(filename);
}