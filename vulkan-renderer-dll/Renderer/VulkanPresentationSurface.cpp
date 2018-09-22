#include "VulkanPresentationSurface.h"



VulkanPresentationSurface::VulkanPresentationSurface()
{
}


VulkanPresentationSurface::~VulkanPresentationSurface()
{
}


bool VulkanPresentationSurface::create(VkInstance instance, HINSTANCE hInstance, HWND hWnd)
{
	this->instance = instance;

	VkWin32SurfaceCreateInfoKHR surface_create_info = {};
	surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_create_info.hinstance = hInstance;
	surface_create_info.hwnd = hWnd;

	VkResult result = vkCreateWin32SurfaceKHR(instance, &surface_create_info, nullptr, &presentation_surface);
	if ((VK_SUCCESS != result) || (VK_NULL_HANDLE == presentation_surface)) {
		std::cout << "Could not create presentation surface." << std::endl;
		return false;
	}
	return true;
}

void VulkanPresentationSurface::shutdown()
{
	if (presentation_surface) {
		std::cout << "Destroy presentation surface\n";
		vkDestroySurfaceKHR(instance, presentation_surface, nullptr);
		presentation_surface = VK_NULL_HANDLE;
	}
}
