#pragma once

#include "DXSample.h"

class DXSample;

class Win32Application
{
public:
	static int Run(DXSample* pSample, HINSTANCE hInstance, int nCmdShow);
	static HWND GetHwnd() { return m_hwnd; }
	static struct WndParams {
		DWORD windowStyleEx;
		DWORD windowStyle;
		WNDCLASSEX* windowClass;
		DXSample* pSample;
		RECT* rect;
	} m_wparams;

protected:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static HWND NewWindow();

private:
	static HWND m_hwnd;
};
