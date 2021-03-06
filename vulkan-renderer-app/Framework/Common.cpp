#include "Common.h"

Common::Common()
{
	is_ready = false;
	vk_renderer = new VulkanRenderer();
}

Common::~Common()
{
	delete vk_renderer;
	shutdown();
}

bool Common::initialize(HINSTANCE hInstance, WNDPROC wndproc, int nCmdShow, PHANDLER_ROUTINE ctrlHandler)
{
	system_set_dpi_awreness();

	system_create_console(ctrlHandler);
	system_create_window(hInstance, wndproc);

	if (win32_vars.hInstance == NULL || win32_vars.hWnd == NULL) {
		return false;
	}

	vk_renderer->initialize(win32_vars.hInstance, win32_vars.hWnd, win32_vars.width, win32_vars.height);

	ShowWindow(win32_vars.hWnd, nCmdShow);
	UpdateWindow(win32_vars.hWnd);

	is_ready = true;

	return true;
}
static float timer = 0.0f;
void Common::render_loop()
{
	system_events_loop();
	if (!IsIconic(win32_vars.hWnd)) {

		auto start_time = std::chrono::high_resolution_clock::now();

		vk_renderer->render();
		vk_renderer->update(timer);

		auto end_time = std::chrono::high_resolution_clock::now();

		update_timer(start_time, end_time);
	}
}

void Common::update_timer(
	std::chrono::time_point<std::chrono::steady_clock> &start_time,
	std::chrono::time_point<std::chrono::steady_clock> &end_time)
{
	frame_counter++;
	
	auto delta_time = std::chrono::duration<double, std::milli>(end_time - start_time).count();
	timer += frame_timer;
	frame_timer = (float)delta_time / 1000.0f;
	fps_timer += (float)delta_time;
	if (fps_timer > 1000.0f) {
		SetWindowText(win32_vars.hWnd, std::to_string(fps_timer).c_str());
		fps_timer = 0.0f;
		frame_counter = 0;
	}
}

void Common::shutdown()
{
	std::cout << "Shutdown...\n";
	vk_renderer->shutdown();
	system_exit();
}

bool Common::system_create_window(HINSTANCE hInstance, WNDPROC wndproc)
{
	win32_vars.hInstance = hInstance;
	system_register_window_class(hInstance, wndproc);
	if (!system_init_instance(hInstance)) {
		return false;
	}
}

ATOM Common::system_register_window_class(HINSTANCE hInstance, WNDPROC wndproc)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = wndproc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = nullptr;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = WIN32_WINDOW_CLASS_NAME;
	wcex.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);

	return RegisterClassEx(&wcex);
}

bool Common::system_init_instance(HINSTANCE hInstance)
{
	HWND hWnd = CreateWindow(WIN32_WINDOW_CLASS_NAME, APPLICATION_NAME, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, win32_vars.width, win32_vars.height, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	win32_vars.hWnd = hWnd;

	return TRUE;
}

BOOL Common::system_console_hanlder(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
		shutdown();
		return(TRUE);
	default:
		return FALSE;
	}
}

void Common::system_create_console(PHANDLER_ROUTINE ctrlHandler) {
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	FILE *stream;
	freopen_s(&stream, "CONOUT$", "w+", stdout);
	freopen_s(&stream, "CONOUT$", "w+", stderr);

	SetConsoleCtrlHandler(ctrlHandler, TRUE);
}

void Common::system_events_loop() {
	MSG msg;

	// pump the message loop
	while (PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE)) {
		if (!GetMessage(&msg, nullptr, 0, 0)) {
			this->shutdown();
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Common::system_destroy_window()
{
	// destroy window
	if (win32_vars.hWnd) {
		std::cout << "destroying window\n";
		DestroyWindow(win32_vars.hWnd);
		win32_vars.hWnd = nullptr;
	}
}

void Common::system_exit()
{
	std::cout << "system exit\n";
	system_destroy_window();
	ExitProcess(0);
}

LRESULT Common::system_handle_messages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
	{
		ValidateRect(hWnd, nullptr);
	}
	break;
	case WM_SIZE:
	{
		if ((is_ready) && wParam != SIZE_MINIMIZED) {
			if ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED)) {
				uint32_t new_width = LOWORD(lParam);
				uint32_t new_height = HIWORD(lParam);
				vk_renderer->resize(new_width, new_height);
			}
		}
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void Common::system_set_dpi_awreness()
{
	HINSTANCE shCoreDll = LoadLibrary("Shcore.dll");
	if (shCoreDll)
	{
		typedef HRESULT(WINAPI* SetProcessDpiAwarenessFuncType)(PROCESS_DPI_AWARENESS);
		SetProcessDpiAwarenessFuncType SetProcessDpiAwarenessFunc = reinterpret_cast<SetProcessDpiAwarenessFuncType>(GetProcAddress(shCoreDll, "SetProcessDpiAwareness"));
		if (SetProcessDpiAwarenessFunc)
		{
			// We only check for E_INVALIDARG because we would get
			// E_ACCESSDENIED if the DPI was already set previously
			// and S_OK means the call was successful
			if (SetProcessDpiAwarenessFunc(PROCESS_PER_MONITOR_DPI_AWARE) == E_INVALIDARG)
			{
				std::cout << "Failed to set process DPI awareness" << std::endl;
			}
			else
			{
				FreeLibrary(shCoreDll);
				return;
			}
		}

		FreeLibrary(shCoreDll);
	}
}