/*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "PropertyGrid.h"
#include "EditorModule.h"
#include "MRenderFX.h"

class RenderFXModule : public EditorModule, public Singleton<RenderFXModule>
{
public:
	RenderFXModule();
	virtual ~RenderFXModule();

	void 
		Layout();
	void 
		Show();
	void 
		Hide();

protected:
	void 
		OnSceneAfterLoad();
	void
		OnUpdate();
	void
		OnResize(int w, int h);

	void
		OnCreateOcean(const MGUI::ClickEvent * e);
	void
		OnKeyFrameChanged(const MGUI::Event * e, int index);
	void
		OnSave(const MGUI::ClickEvent * e);

protected:
	MGUI::Layout * mLayout;
	RenderFX * mRenderFX;

	MGUI::Layout * mPropertyPanel;
	PropertyGrid * mPropertyGrid;

	MGUI::Layout * mTestLayout;
	MGUI::ImageBox * mImageBox;
};
