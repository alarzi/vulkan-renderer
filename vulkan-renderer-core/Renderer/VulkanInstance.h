#pragma once

#include <cassert>
#include <sstream>
#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>

#include "VulkanTools.h"
#include "../Framework/Properties.h"

class VulkanInstance
{
public:
	VulkanInstance();
	~VulkanInstance();

	bool					create();
	void					shutdown();

	operator VkInstance() { return instance; };

private:
	/** @brief Instance */
	VkInstance				instance;
	bool					create_instance();
};
