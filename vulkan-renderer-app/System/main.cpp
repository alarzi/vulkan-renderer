#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../Framework/Common.h"

std::unique_ptr<Common> common(new Common());

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return common->system_handle_messages(hWnd, message, wParam, lParam);
}

BOOL CtrlHandler(DWORD fdwCtrlType)
{
	return common->system_console_hanlder(fdwCtrlType);
}

int main()
{
	try {
		common->initialize((HINSTANCE)::GetModuleHandle(NULL), WndProc, SW_SHOWDEFAULT, (PHANDLER_ROUTINE)CtrlHandler);
		while (1) {
			common->render_loop();
		}
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

//int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
//{
//	try {
//		common->initialize(hInstance, WndProc, nCmdShow, (PHANDLER_ROUTINE)CtrlHandler);
//		while (1) {
//			common->render_loop();
//		}
//	}
//	catch (const std::exception& e) {
//		std::cerr << e.what() << std::endl;
//		return EXIT_FAILURE;
//	}
//
//	return EXIT_SUCCESS;
//}
