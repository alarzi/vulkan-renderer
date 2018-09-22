#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../Framework/Common.h"

Common * common = new Common();

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return common->system_handle_messages(hWnd, message, wParam, lParam);
}

BOOL CtrlHandler(DWORD fdwCtrlType)
{
	return common->system_console_hanlder(fdwCtrlType);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	common->initialize(hInstance, WndProc, nCmdShow, (PHANDLER_ROUTINE)CtrlHandler);

	while (1) {
		common->render_loop();
	}

	delete common;
	return 0;
}
