#include "VulkanExports.h"
#include "../Renderer/VulkanRenderer.h"

//extern "C" VULKAN_RENDERER_API void vk_initialize(HINSTANCE hInstance, HWND hWnd, int width, int height)
//{
//	if (!hInstance) {
//		hInstance = (HINSTANCE)::GetModuleHandle(NULL);
//	}
//	VulkanRenderer::get_instance()->initialize(hInstance, (HWND)hWnd, width, height);
//}
//
//extern "C" VULKAN_RENDERER_API void vk_resize(int width, int height)
//{
//	VulkanRenderer::get_instance()->resize(width, height);
//}
//
//extern "C" VULKAN_RENDERER_API void vk_render()
//{
//	VulkanRenderer::get_instance()->render();
//}
//
//extern "C" VULKAN_RENDERER_API void vk_shutdown()
//{
//	VulkanRenderer::get_instance()->shutdown();
//}