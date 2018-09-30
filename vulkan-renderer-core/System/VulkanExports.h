#pragma once

#include <Windows.h>
#include "../Framework/Properties.h"

extern "C" VULKAN_RENDERER_API void vk_initialize(HINSTANCE hInstance, HWND hWnd, int width, int height);

extern "C" VULKAN_RENDERER_API void vk_resize(int width, int height);

extern "C" VULKAN_RENDERER_API void vk_render();

extern "C" VULKAN_RENDERER_API void vk_shutdown();