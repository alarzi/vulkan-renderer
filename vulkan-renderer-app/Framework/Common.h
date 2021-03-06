#pragma once

#include <ShellScalingAPI.h>

#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <io.h>
#include <fcntl.h>
#include <chrono>
#include <string>

#include "Properties.h"

#include "Renderer/VulkanRenderer.h"
#include "System/VulkanExports.h"

class Common
{

public:
	Common();
	~Common();

	bool initialize(HINSTANCE hInstance, WNDPROC wndproc, int nCmdShow, PHANDLER_ROUTINE ctrlHandler);
	void render_loop();
	void shutdown();

	LRESULT system_handle_messages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	BOOL system_console_hanlder(DWORD fdwCtrlType);

	struct {
		HWND			hWnd;
		HINSTANCE		hInstance;
		uint32_t		width = DEFAULT_WINDOW_WIDTH;
		uint32_t		height = DEFAULT_WINDOW_HEIGHT;
	} win32_vars;

private:
	VulkanRenderer * vk_renderer;
	bool is_ready;

	uint32_t frame_counter = 0;
	float frame_timer;
	float fps_timer;

	void system_create_console(PHANDLER_ROUTINE ctrlHandler);
	void system_set_dpi_awreness();
	void system_events_loop();
	void system_exit();
	void system_destroy_window();

	bool system_create_window(HINSTANCE hInstance, WNDPROC wndproc);

	ATOM system_register_window_class(HINSTANCE hInstance, WNDPROC wndproc);
	bool system_init_instance(HINSTANCE hInstance);

	void update_timer(std::chrono::time_point<std::chrono::steady_clock> &start_time, std::chrono::time_point<std::chrono::steady_clock> &end_time);
};

