#include "stdafx.h"
#include "MGUI_Cursor.h"
#include "MGUI_Input.h"

#include "App.h"

MGUI::Cursor * gCursor = NULL;
App * gApp = NULL;

LRESULT CALLBACK WndProc(HWND hWnd,UINT iMsg,WPARAM wParam,LPARAM lParam) 
{
	if (gApp == NULL)
		return ::DefWindowProc(hWnd, iMsg, wParam, lParam);

	switch(iMsg)
	{
	case WM_CREATE:
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_SETCURSOR:
		gCursor->Draw();
		break;

	case WM_LBUTTONDOWN:
		if (MGUI::InputManager::Instance())
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);

			gApp->InjectMouseDown(InputCode::MKC_LEFT, (float)x, (float)y);
		}
		break;

	case WM_LBUTTONUP:
		if (MGUI::InputManager::Instance())
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);

			gApp->InjectMouseUp(InputCode::MKC_LEFT, (float)x, (float)y);
		}
		break;

	case WM_RBUTTONDOWN:
		if (MGUI::InputManager::Instance())
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);

			gApp->InjectMouseDown(InputCode::MKC_RIGHT, (float)x, (float)y);
		}
		break;

	case WM_RBUTTONUP:
		if (MGUI::InputManager::Instance())
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);

			gApp->InjectMouseUp(InputCode::MKC_RIGHT, (float)x, (float)y);
		}
		break;

	case WM_MOUSEMOVE:
		if (MGUI::InputManager::Instance())
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);

			gApp->InjectMouseMove((float)x, (float)y);
		}
		break;

	case WM_KEYDOWN:
		{
			bool repeat = (lParam & (1 >> 30)) != 0;
			if (!repeat)
			{
				int scan_code = MGUI::Input::VirtualKeyToScanCode(wParam);
				int text = MGUI::Input::VirtualKeyToText(wParam);

				gApp->InjectKeyDown(scan_code, (uchar_t)text);
			}
		}
		break;

	case WM_KEYUP:
		{
			int scan_code = MGUI::Input::VirtualKeyToScanCode(wParam);

			gApp->InjectKeyUp(scan_code);
		}
		break;

	case WM_IME_CHAR:
		{
			int text = 0;
#ifdef _UNICODE
			text = wParam;
#else
			char mbstr[3];
			BYTE hiByte = wParam >> 8;
			BYTE loByte = wParam & 0x000000FF;
			if (hiByte == 0)
			{
				mbstr[0] = loByte;
				mbstr[1] = '\0';
			}
			else
			{
				mbstr[0] = hiByte;
				mbstr[1] = loByte;
				mbstr[2] = '\0';
			}

			wchar_t wstr[2];
			/*int num = */MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, mbstr, -1, wstr, _countof(wstr));
			text = wstr[0];
#endif // _UNICODE

			gApp->InjectKeyDown(InputCode::KC_NONE, (uchar_t)text);
		}
		break;

	case WM_SIZE:
		if (1)
		{
			static bool s_paused = false;

			if (wParam == SIZE_MINIMIZED)
			{
				gApp->Pause();
				s_paused = true;
			}
			else if (s_paused)
			{
				gApp->Resume();
				s_paused = false;
			}
		}

		if (wParam != SIZE_MINIMIZED)
		{
			RECT rc;

			GetClientRect(hWnd, &rc);

			int w = rc.right - rc.left;
			int h = rc.bottom - rc.top;

			gApp->Resize(w, h);
		}
		break;

	case WM_DROPFILES:
		{
			TCHAR meshFile[MAX_PATH] = { 0 };
			HDROP hDrop = (HDROP)wParam;
			int len = DragQueryFile(hDrop, 0, meshFile, MAX_PATH);

			if (len > 0 && gApp != NULL)
			{
				gApp->OnDragFile(meshFile);
			}
		}
		break;

	default:
		break;
	}

	return ::DefWindowProc(hWnd, iMsg, wParam, lParam); 
}

static const char * WIN32_CLASS_NAME = "Myway Game";
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF));

	WNDCLASS wc;
	wc.cbClsExtra = 0; 
	wc.cbWndExtra = 0; 
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hCursor= NULL; 
	wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION); 
	wc.hInstance = hInstance; 
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = WIN32_CLASS_NAME; 
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	DWORD style = WS_OVERLAPPEDWINDOW;
	POINT pt = { CW_USEDEFAULT, CW_USEDEFAULT };
	SIZE sz = { CW_USEDEFAULT, CW_USEDEFAULT };

	RegisterClass(&wc);

#ifndef IN_EDITOR
	sz.cx = 1280;
	sz.cy = 720;
#endif

	HWND hWnd = CreateWindow(WIN32_CLASS_NAME, "Window", style, 
		pt.x, pt.y, sz.cx, sz.cy,
		NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, SW_MAXIMIZE);

	RECT rc;
	GetClientRect(hWnd, &rc);
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

	try {

		gCursor = new MGUI::Cursor;

		CreateApplication(&gApp);
		d_assert (gApp != NULL);

		gApp->SetCachePath("Cache");

		gApp->Init(hInstance, hWnd, width, height);

		MSG msg;
		memset(&msg,0,sizeof(msg));
		while(msg.message != WM_QUIT)
		{
			if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				gApp->Update();
			}
		}

		gApp->Shutdown();
		delete gApp;

		delete gCursor;
	}
	catch(...) {
		d_assert(0);
	}

	UnregisterClass(WIN32_CLASS_NAME, hInstance);

	return 0;
}