//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "Win32Application.h"

HWND Win32Application::m_hwnd = nullptr;
Win32Application::WndParams Win32Application::m_wparams;

HWND Win32Application::NewWindow() {
	RECT windowRect = { 0, 0, static_cast<LONG>(m_wparams.pSample->GetWidth()), static_cast<LONG>(m_wparams.pSample->GetHeight()) };
	return CreateWindowEx(
		m_wparams.windowStyleEx,
		m_wparams.windowClass->lpszClassName,
		m_wparams.pSample->GetTitle(),
		m_wparams.windowStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,		// We have no parent window.
		nullptr,		// We aren't using menus.
		m_wparams.windowClass->hInstance,
		m_wparams.pSample
	);
}

int Win32Application::Run(DXSample* pSample, HINSTANCE hInstance, int nCmdShow)
{
	// Parse the command line parameters
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	pSample->ParseCommandLineArgs(argv, argc);
	LocalFree(argv);

	// Initialize the window class.
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = L"DXSampleClass";
	//windowClass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0)); //due to DComp its pointless to use it
	RegisterClassEx(&windowClass);

	RECT windowRect = { 0, 0, static_cast<LONG>(pSample->GetWidth()), static_cast<LONG>(pSample->GetHeight()) };

	m_wparams = {
		WS_EX_NOREDIRECTIONBITMAP | WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
		//WS_EX_COMPOSITED | WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
		//WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
		WS_POPUP,
		&windowClass,
		pSample,
		&windowRect
	};
	AdjustWindowRect(&windowRect, m_wparams.windowStyle, FALSE);

	// Create the window and store a handle to it.
	m_hwnd = NewWindow();

	if (!m_hwnd) {
		//if WS_EX_NOREDIRECTIONBITMAP failed
		m_wparams.windowStyleEx = WS_EX_COMPOSITED | WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED; //TODO: working but lags when cursor moved inside(not ok)
		// Create the window again.
		m_hwnd = NewWindow();
	}
	
	if (!m_hwnd) {
		//if WS_EX_COMPOSITED failed
		m_wparams.windowStyleEx = WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED;
		// Create the window again.
		m_hwnd = NewWindow();
	}

	if (!m_hwnd)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		//hr = HRESULT_FROM_WIN32(GetLastError());
	}

	SetLayeredWindowAttributes(m_hwnd,0,0,LWA_COLORKEY); // its anyway will be transparent due to DComposition
    
	// Initialize the sample. OnInit is defined in each child-implementation of DXSample.
	pSample->OnInit();

	ShowWindow(m_hwnd, nCmdShow);

	// Main sample loop.
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		// Process any messages in the queue.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	pSample->OnDestroy();

	// Return this part of the WM_QUIT message to Windows.
	return static_cast<char>(msg.wParam);
}

// Main message handler for the sample.
LRESULT CALLBACK Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	DXSample* pSample = reinterpret_cast<DXSample*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (message)
	{
	case WM_CREATE:
		{
			// Save the DXSample* passed in to CreateWindow.
			LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
		}
		return 0;

	case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
        }
		else if (pSample)
		{
			pSample->OnKeyDown(static_cast<UINT8>(wParam));
		}
		return 0;

	case WM_KEYUP:
		if (pSample)
		{
			pSample->OnKeyUp(static_cast<UINT8>(wParam));
		}
		return 0;

	case WM_PAINT:
		if (pSample)
		{
			pSample->OnUpdate();
			pSample->OnRender();
		}
		return 0;

    case WM_NCHITTEST:
        return HTCAPTION;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	// Handle any messages the switch statement didn't.
	return DefWindowProc(hWnd, message, wParam, lParam);
}
