#pragma once

#define APPLICATION_NAME				"vk rendering engine"
#define	WIN32_WINDOW_CLASS_NAME			"VK_RENDERING_ENGINE_WINDOW"

#define DEFAULT_WINDOW_WIDTH			1280
#define DEFAULT_WINDOW_HEIGHT			720

#define ENABLE_DEBUG_LAYERS

#define MULTISAMPLE_LEVEL				VK_SAMPLE_COUNT_1_BIT

#ifdef VULKAN_RENDERER_EXPORTS
#define VULKAN_RENDERER_API __declspec(dllexport)
#else
#define VULKAN_RENDERER_API __declspec(dllimport)
#endif