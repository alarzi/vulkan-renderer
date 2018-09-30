#include "VulkanShader.h"

using namespace std::string_literals;

VulkanShader::VulkanShader()
{
}


VulkanShader::~VulkanShader()
{
}

VkShaderModule VulkanShader::load(VkDevice device, std::string filename)
{
	// Load the content of the shader file
	auto shader_content = load_file(filename);

	// Create a new shader module that will be used for pipeline creation
	VkShaderModuleCreateInfo module_create_info = {};
	module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	module_create_info.codeSize = shader_content.size();
	module_create_info.pCode = reinterpret_cast<const uint32_t*>(shader_content.data());

	VkShaderModule shaderModule;
	VK_CHECK_RESULT(vkCreateShaderModule(device, &module_create_info, nullptr, &shaderModule));

	return shaderModule;
}

std::vector<char> VulkanShader::load_file(std::string &filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::in | std::ios::ate);

	if (!file.is_open()) {
		throw std::runtime_error("Error: Could not open shader file '"s + filename + "'"s);
	}

	size_t file_size = file.tellg();
	file.seekg(0, std::ios::beg);

	// Copy file contents into a buffer
	std::vector<char> file_buffer(file_size);
	file.read(file_buffer.data(), file_size);
	file.close();

	if (file_size <= 0 || file_buffer.empty()) {
		throw std::runtime_error("The shader is empty");
	}

	return file_buffer;
}
