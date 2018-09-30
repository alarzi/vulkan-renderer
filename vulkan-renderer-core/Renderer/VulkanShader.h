#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>
#include <exception>

#include <vulkan/vulkan.h>

#include "VulkanTools.h"

class VulkanShader
{
public:
	VulkanShader();
	~VulkanShader();

	VkShaderModule load(VkDevice device, std::string filename);
	
private:
	std::vector<char> load_file(std::string &filename);
};

