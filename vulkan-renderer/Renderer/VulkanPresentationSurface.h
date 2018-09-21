#pragma once

#include <iostream>

#include <vulkan/vulkan.h>

class VulkanPresentationSurface
{
public:
	VulkanPresentationSurface();
	~VulkanPresentationSurface();

	VkSurfaceKHR			presentation_surface;

	bool					create(VkInstance instance, HINSTANCE hInstance, HWND hWnd);
	void					shutdown();

	operator VkSurfaceKHR() { return presentation_surface; };

private:
	VkInstance				instance;
};

