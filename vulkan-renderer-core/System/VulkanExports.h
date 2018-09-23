#pragma once

#include <Windows.h>

#ifdef VULKAN_RENDERER_EXPORTS
#define VULKAN_RENDERER_API __declspec(dllexport)
#else
#define VULKAN_RENDERER_API __declspec(dllimport)
#endif

extern "C" VULKAN_RENDERER_API void vk_initialize(HINSTANCE hInstance, HWND hWnd, int width, int height);

extern "C" VULKAN_RENDERER_API void vk_resize(int width, int height);

extern "C" VULKAN_RENDERER_API void vk_render();

extern "C" VULKAN_RENDERER_API void vk_shutdown();