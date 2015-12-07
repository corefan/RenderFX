#include "RenderFXPlugin.h"
#include "MainFrame.h"
#include "RenderFXModule.h"

RenderFXPlugin::~RenderFXPlugin()
{
}

void RenderFXPlugin::Init()
{
	MainFrame::Instance()->AddModule(new RenderFXModule);
}

void RenderFXPlugin::Shutdown()
{
}