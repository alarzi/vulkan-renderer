#pragma once

#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>

class VulkanPresentationSurface
{
public:
	VulkanPresentationSurface();
	~VulkanPresentationSurface();

	VkSurfaceKHR			presentation_surface;

	bool					create(VkInstance instance, HINSTANCE hInstance, HWND hWnd);
	void					shutdown();

	bool					get_capabilities(VkPhysicalDevice physical_device, VkSurfaceCapabilitiesKHR& surface_capabilities);
	bool					get_format(VkPhysicalDevice physical_device, VkSurfaceFormatKHR desired_surface_format, VkFormat& format, VkColorSpaceKHR& color_space);

	operator VkSurfaceKHR() { return presentation_surface; };

private:
	VkInstance				instance;
};

