#include "VulkanRenderer.h"

VulkanRenderer* VulkanRenderer::_instance = nullptr;
VulkanRenderer* VulkanRenderer::get_instance()
{
	if (!_instance) {
		_instance = new VulkanRenderer;
	}
	return _instance;
}

VulkanRenderer::VulkanRenderer()
{
	is_ready = false;
}

VulkanRenderer::~VulkanRenderer()
{
	shutdown();
}

/**
* TODO
*
* @param hInstance TODO
* @param hWnd TODO
*
*/
bool VulkanRenderer::initialize(HINSTANCE hInstance, HWND hWnd, uint32_t width, uint32_t height)
{
	instance.create();
	presentation_surface.create(instance, hInstance, hWnd);
	device.create(instance, presentation_surface);

	swapchain.create(instance, device, presentation_surface, &width, &height);

	create_depth_buffer(width, height, &depth_buffer);

	create_command_pool(device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, device.graphics_queue_family_index, &command_pool);
	allocate_command_buffer(device, command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, swapchain.images.size(), command_buffers);

	create_descriptor_set_layout(&descriptor_set_layout);
	create_pipeline_layout(&pipeline_layout);

	create_render_pass(&render_pass);
	create_frame_buffer(width, height, frame_buffers);

	create_uniform_buffer(&uniform_buffer);
	update_uniform_buffer(width, height, &uniform_buffer);
	create_vertex_buffer(&vertex_buffer, &index_buffer);
	
	create_descriptor_pool(&descriptor_pool);
	create_descriptor_set(&descriptor_set);

	create_pipeline_cache(&pipeline_cache);
	create_graphics_pipeline(&graphics_pipeline);

	create_semaphores();
	create_command_buffer(width, height, command_buffers);

	is_ready = true;

	return true;
}

void VulkanRenderer::render()
{
	if (!is_ready)
		return;

	swapchain.acquire_next_image_index(image_acquired_semaphore, (VkFence)nullptr, &current_buffer);

	VK_CHECK_RESULT(vkWaitForFences(device, 1, &draw_fences[current_buffer], VK_TRUE, UINT64_MAX));
	VK_CHECK_RESULT(vkResetFences(device, 1, &draw_fences[current_buffer]));

	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	// The submit info structure specifices a command buffer queue submission batch
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pWaitDstStageMask = &waitStageMask;
	submitInfo.pWaitSemaphores = &image_acquired_semaphore;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &render_complete_semaphore;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pCommandBuffers = &command_buffers[current_buffer];
	submitInfo.commandBufferCount = 1;

	VK_CHECK_RESULT(vkQueueSubmit(device.present_queue, 1, &submitInfo, draw_fences[current_buffer]));

	VK_CHECK_RESULT(swapchain.queue_present(device.present_queue, current_buffer, render_complete_semaphore));
}

void VulkanRenderer::resize(uint32_t width, uint32_t height)
{
	if (!is_ready) {
		return;
	}
	is_ready = false;

	vkDeviceWaitIdle(device);

	// Recreate the swapchain
	swapchain.create(instance, device, presentation_surface, &width, &height);

	// Recreate the frame buffers
	vkDestroyImageView(device, depth_buffer.view, nullptr);
	vkDestroyImage(device, depth_buffer.image, nullptr);
	vkFreeMemory(device, depth_buffer.memory, nullptr);
	create_depth_buffer(width, height, &depth_buffer);
	for (uint32_t i = 0; i < frame_buffers.size(); i++) {
		vkDestroyFramebuffer(device, frame_buffers[i], nullptr);
	}
	create_frame_buffer(width, height, frame_buffers);

	// Command buffers need to be recreated as they may store
	// references to the recreated frame buffer
	vkFreeCommandBuffers(device, command_pool, static_cast<uint32_t>(command_buffers.size()), command_buffers.data());
	allocate_command_buffer(device, command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, swapchain.images.size(), command_buffers);
	create_command_buffer(width, height, command_buffers);

	vkDeviceWaitIdle(device);

	update_uniform_buffer(width, height, &uniform_buffer);

	is_ready = true;
}


void VulkanRenderer::shutdown()
{
	is_ready = false;

	std::cout << "Destroy pipeline\n";
	vkDestroyPipeline(device, graphics_pipeline, nullptr);

	std::cout << "Destroy buffer\n";
	vkDestroyBuffer(device, vertex_buffer.buffer, nullptr);
	vkFreeMemory(device, vertex_buffer.memory, nullptr);

	std::cout << "Destroy frame buffers\n";
	for (uint32_t i = 0; i < frame_buffers.size(); i++) {
		vkDestroyFramebuffer(device, frame_buffers[i], nullptr);
	}

	std::cout << "Destroy render pass\n";
	vkDestroyRenderPass(device, render_pass, nullptr);

	std::cout << "Destroy semaphore\n";
	vkDestroySemaphore(device, image_acquired_semaphore, nullptr);

	vkDestroyDescriptorPool(device, descriptor_pool, nullptr);

	vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);
	vkDestroyPipelineLayout(device, pipeline_layout, nullptr);

	vkDestroyBuffer(device, uniform_buffer.buffer, nullptr);
	vkFreeMemory(device, uniform_buffer.memory, nullptr);

	vkFreeCommandBuffers(device, command_pool, static_cast<uint32_t>(command_buffers.size()), command_buffers.data());
	vkDestroyCommandPool(device, command_pool, nullptr);

	vkDestroyImageView(device, depth_buffer.view, nullptr);
	vkDestroyImage(device, depth_buffer.image, nullptr);
	vkFreeMemory(device, depth_buffer.memory, nullptr);

	swapchain.shutdown();
	presentation_surface.shutdown();
	device.shutdown();
	instance.shutdown();
}

bool VulkanRenderer::create_buffer(
	VkDevice logical_device,
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkBuffer & buffer)
{
	VkBufferCreateInfo buffer_create_info = {};
	buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.usage = usage;
	buffer_create_info.size = size;
	buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buffer_create_info.queueFamilyIndexCount = 0;
	buffer_create_info.pQueueFamilyIndices = nullptr;

	VK_CHECK_RESULT(vkCreateBuffer(logical_device, &buffer_create_info, nullptr, &buffer));

	return true;
}

bool VulkanRenderer::create_command_pool(
	VkDevice logical_device,
	VkCommandPoolCreateFlags parameters,
	uint32_t queue_family,
	VkCommandPool* command_pool)
{
	VkCommandPoolCreateInfo command_pool_create_info = {};
	command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_create_info.flags = parameters;
	command_pool_create_info.queueFamilyIndex = queue_family;

	VK_CHECK_RESULT(vkCreateCommandPool(logical_device, &command_pool_create_info, nullptr, command_pool));

	return true;
}

bool VulkanRenderer::allocate_command_buffer(
	VkDevice logical_device,
	VkCommandPool command_pool,
	VkCommandBufferLevel level,
	uint32_t count,
	std::vector<VkCommandBuffer> & command_buffers)
{
	VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
	command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.commandPool = command_pool;
	command_buffer_allocate_info.level = level;
	command_buffer_allocate_info.commandBufferCount = count;

	command_buffers.resize(count);

	VK_CHECK_RESULT(vkAllocateCommandBuffers(logical_device, &command_buffer_allocate_info, command_buffers.data()));

	return true;
}

bool VulkanRenderer::create_depth_buffer(const uint32_t width, const uint32_t height, DepthBuffer* depth_buffer)
{
	// specify the depth buffer format
	VkFormat depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
	bool valid_depth_format = vks::tools::get_supported_depth_format(device.physical_device, &depth_format);
	depth_buffer->format = depth_format;

	VkImageCreateInfo depth_image_create_info = {};
	depth_image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	depth_image_create_info.imageType = VK_IMAGE_TYPE_2D;
	depth_image_create_info.format = depth_format;
	depth_image_create_info.extent.width = width;
	depth_image_create_info.extent.height = height;
	depth_image_create_info.extent.depth = 1;
	depth_image_create_info.mipLevels = 1;
	depth_image_create_info.arrayLayers = 1;
	depth_image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_image_create_info.usage = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
	depth_image_create_info.queueFamilyIndexCount = 0;
	depth_image_create_info.pQueueFamilyIndices = nullptr;
	depth_image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// check tiling mode
	VkFormatProperties format_properties;
	vkGetPhysicalDeviceFormatProperties(device.physical_device, depth_format, &format_properties);
	if (format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		depth_image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
	}
	else if (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		depth_image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	}
	else {
		std::cout << "VK_FORMAT_D16_UNORM not supported.\n";
		return false;
	}

	// Create image
	VK_CHECK_RESULT(vkCreateImage(device, &depth_image_create_info, nullptr, &depth_buffer->image));

	// use the memory properties to determine the type of memory required
	VkMemoryRequirements depth_memory_requirements;
	vkGetImageMemoryRequirements(device, depth_buffer->image, &depth_memory_requirements);

	VkMemoryAllocateInfo depth_buffer_memory = {};
	depth_buffer_memory.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	depth_buffer_memory.allocationSize = depth_memory_requirements.size;
	depth_buffer_memory.memoryTypeIndex = get_memory_type_index(depth_memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// Allocate memory
	VK_CHECK_RESULT(vkAllocateMemory(device, &depth_buffer_memory, nullptr, &depth_buffer->memory));

	// Bind memory
	VK_CHECK_RESULT(vkBindImageMemory(device, depth_buffer->image, depth_buffer->memory, 0));

	// Create the image view
	VkImageViewCreateInfo depth_image_view_create_info = {};
	depth_image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depth_image_view_create_info.image = depth_buffer->image;
	depth_image_view_create_info.format = depth_format;
	depth_image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	depth_image_view_create_info.subresourceRange.baseMipLevel = 0;
	depth_image_view_create_info.subresourceRange.levelCount = 1;
	depth_image_view_create_info.subresourceRange.baseArrayLayer = 0;
	depth_image_view_create_info.subresourceRange.layerCount = 1;
	depth_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;

	VK_CHECK_RESULT(vkCreateImageView(device, &depth_image_view_create_info, nullptr, &depth_buffer->view));

	return true;
}

bool VulkanRenderer::create_descriptor_set_layout(VkDescriptorSetLayout* descriptor_set_layout)
{
	VkDescriptorSetLayoutBinding layout_binding = {};
	layout_binding.binding = 0;
	layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layout_binding.descriptorCount = 1;
	layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layout_binding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
	descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_layout.bindingCount = 1;
	descriptor_layout.pBindings = &layout_binding;

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptor_layout, nullptr, descriptor_set_layout));

	return true;
}

void VulkanRenderer::create_pipeline_layout(VkPipelineLayout* pipeline_layout)
{
	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
	pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_info.pushConstantRangeCount = 0;
	pipeline_layout_create_info.pPushConstantRanges = nullptr;
	pipeline_layout_create_info.setLayoutCount = 1;
	pipeline_layout_create_info.pSetLayouts = &descriptor_set_layout;

	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, pipeline_layout));
}

void VulkanRenderer::create_render_pass(VkRenderPass* render_pass)
{
	std::array<VkAttachmentDescription, 2> attachments{};
	attachments[0].format = swapchain.image_format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	attachments[1].format = depth_buffer.format;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference color_reference = {};
	color_reference.attachment = 0;
	color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_reference = {};
	depth_reference.attachment = 1;
	depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription sub_pass_description = {};
	sub_pass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	sub_pass_description.inputAttachmentCount = 0;
	sub_pass_description.pInputAttachments = nullptr;
	sub_pass_description.colorAttachmentCount = 1;
	sub_pass_description.pColorAttachments = &color_reference;
	sub_pass_description.pResolveAttachments = nullptr;
	sub_pass_description.pDepthStencilAttachment = &depth_reference;
	sub_pass_description.preserveAttachmentCount = 0;
	sub_pass_description.pPreserveAttachments = nullptr;

	std::array<VkSubpassDependency, 2> dependencies;

	// First dependency at the start of the renderpass
	// Does the transition from final to initial layout 
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;								// Producer of the dependency 
	dependencies[0].dstSubpass = 0;													// Consumer is our single subpass that will wait for the execution depdendency
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Second dependency at the end the renderpass
	// Does the transition from the initial to the final layout
	dependencies[1].srcSubpass = 0;													// Producer of the dependency is our single subpass
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;								// Consumer are all commands outside of the renderpass
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo render_pass_create_info = {};
	render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
	render_pass_create_info.pAttachments = attachments.data();
	render_pass_create_info.subpassCount = 1;
	render_pass_create_info.pSubpasses = &sub_pass_description;
	render_pass_create_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
	render_pass_create_info.pDependencies = dependencies.data();

	VK_CHECK_RESULT(vkCreateRenderPass(device, &render_pass_create_info, nullptr, render_pass));
}

void VulkanRenderer::create_frame_buffer(const uint32_t &width, const uint32_t &height, std::vector<VkFramebuffer>& frame_buffers)
{
	frame_buffers.resize(swapchain.images.size());
	for (uint32_t i = 0; i < frame_buffers.size(); i++)
	{
		std::array<VkImageView, 2> frame_buffers_attachments;
		frame_buffers_attachments[0] = swapchain.images[i].view;
		frame_buffers_attachments[1] = depth_buffer.view;

		VkFramebufferCreateInfo frame_buffer_create_info = {};
		frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frame_buffer_create_info.renderPass = render_pass;
		frame_buffer_create_info.attachmentCount = static_cast<uint32_t>(frame_buffers_attachments.size());
		frame_buffer_create_info.pAttachments = frame_buffers_attachments.data();
		frame_buffer_create_info.width = width;
		frame_buffer_create_info.height = height;
		frame_buffer_create_info.layers = 1;

		VK_CHECK_RESULT(vkCreateFramebuffer(device, &frame_buffer_create_info, nullptr, &frame_buffers[i]));
	}
}

void VulkanRenderer::create_uniform_buffer(VulkanBuffer* uniform_buffer)
{
	create_buffer(device, sizeof(mvp_matrix), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uniform_buffer->buffer);

	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements(device, uniform_buffer->buffer, &memory_requirements);

	VkMemoryAllocateInfo memory_allocate_info = {};
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize = memory_requirements.size;
	memory_allocate_info.memoryTypeIndex = get_memory_type_index(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VK_CHECK_RESULT(vkAllocateMemory(device, &memory_allocate_info, nullptr, &uniform_buffer->memory));
	VK_CHECK_RESULT(vkBindBufferMemory(device, uniform_buffer->buffer, uniform_buffer->memory, 0));
	uniform_buffer->buffer_info.buffer = uniform_buffer->buffer;
	uniform_buffer->buffer_info.offset = 0;
	uniform_buffer->buffer_info.range = sizeof(mvp_matrix);
}

void VulkanRenderer::update_uniform_buffer(const uint32_t &width, const uint32_t &height, VulkanBuffer* uniform_buffer)
{
	mvp_matrix.projection = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.1f, 256.0f);
	mvp_matrix.view = glm::lookAt(
		glm::vec3(0.0f, 0.0f, -10.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	mvp_matrix.model = glm::mat4(1.0f);

	uint8_t* data;
	VK_CHECK_RESULT(vkMapMemory(device, uniform_buffer->memory, 0, sizeof(mvp_matrix), 0, (void**)&data));
	memcpy(data, &mvp_matrix, sizeof(mvp_matrix));
	vkUnmapMemory(device, uniform_buffer->memory);
}

void VulkanRenderer::create_vertex_buffer(VulkanBuffer* vertex_buffer, VulkanIndexBuffer* index_buffer)
{
	std::vector<Vertex> vertex_buffer_data =
	{
		{ { 1.0f,  1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
	{ { -1.0f,  1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
	{ { 0.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
	};
	uint32_t vertex_buffer_size = static_cast<uint32_t>(vertex_buffer_data.size()) * sizeof(Vertex);

	std::vector<uint32_t> index_buffer_data = { 0, 1, 2 };
	index_buffer->count = static_cast<uint32_t>(index_buffer_data.size());
	uint32_t index_buffer_size = index_buffer->count * sizeof(uint32_t);

	uint8_t * data;

	// vertex buffer
	VkBufferCreateInfo vertex_buffer_create_info = {};
	vertex_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertex_buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertex_buffer_create_info.size = vertex_buffer_size;
	VK_CHECK_RESULT(vkCreateBuffer(device, &vertex_buffer_create_info, nullptr, &vertex_buffer->buffer));

	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements(device, vertex_buffer->buffer, &memory_requirements);

	VkMemoryAllocateInfo memory_allocate_info = {};
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize = memory_requirements.size;
	memory_allocate_info.memoryTypeIndex = get_memory_type_index(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(device, &memory_allocate_info, nullptr, &(vertex_buffer->memory)));
	VK_CHECK_RESULT(vkMapMemory(device, vertex_buffer->memory, 0, memory_requirements.size, 0, (void **)&data));
	memcpy(data, vertex_buffer_data.data(), vertex_buffer_size);
	vkUnmapMemory(device, vertex_buffer->memory);
	VK_CHECK_RESULT(vkBindBufferMemory(device, vertex_buffer->buffer, vertex_buffer->memory, 0));

	// index buffer
	VkBufferCreateInfo index_buffer_create_info = {};
	index_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	index_buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	index_buffer_create_info.size = index_buffer_size;
	VK_CHECK_RESULT(vkCreateBuffer(device, &index_buffer_create_info, nullptr, &index_buffer->buffer));
	vkGetBufferMemoryRequirements(device, index_buffer->buffer, &memory_requirements);
	memory_allocate_info.allocationSize = memory_requirements.size;
	memory_allocate_info.memoryTypeIndex = get_memory_type_index(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(device, &memory_allocate_info, nullptr, &(index_buffer->memory)));
	VK_CHECK_RESULT(vkMapMemory(device, index_buffer->memory, 0, memory_requirements.size, 0, (void **)&data));
	memcpy(data, index_buffer_data.data(), index_buffer_size);
	vkUnmapMemory(device, index_buffer->memory);
	VK_CHECK_RESULT(vkBindBufferMemory(device, index_buffer->buffer, index_buffer->memory, 0));
}

bool VulkanRenderer::create_descriptor_pool(VkDescriptorPool *descriptor_pool)
{
	VkDescriptorPoolSize type_count[1];
	type_count[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	type_count[0].descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptor_pool_create_info = {};
	descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool_create_info.maxSets = 1;
	descriptor_pool_create_info.poolSizeCount = 1;
	descriptor_pool_create_info.pPoolSizes = type_count;

	VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptor_pool_create_info, nullptr, descriptor_pool));

	return true;
}

void VulkanRenderer::create_descriptor_set(VkDescriptorSet* descriptor_set)
{
	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = descriptor_pool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &descriptor_set_layout;

	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &alloc_info, descriptor_set));

	VkWriteDescriptorSet write_descriptor_set = {};

	write_descriptor_set = {};
	write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor_set.dstSet = *descriptor_set;
	write_descriptor_set.descriptorCount = 1;
	write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write_descriptor_set.pBufferInfo = &uniform_buffer.buffer_info;
	write_descriptor_set.dstArrayElement = 0;
	write_descriptor_set.dstBinding = 0;

	vkUpdateDescriptorSets(device, 1, &write_descriptor_set, 0, nullptr);
}

void VulkanRenderer::create_pipeline_cache(VkPipelineCache* pipeline_cache)
{
	VkPipelineCacheCreateInfo pipeline_cache_create_info;
	pipeline_cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipeline_cache_create_info.initialDataSize = 0;
	pipeline_cache_create_info.pInitialData = nullptr;
	VK_CHECK_RESULT(vkCreatePipelineCache(device, &pipeline_cache_create_info, nullptr, pipeline_cache));
}

void VulkanRenderer::create_graphics_pipeline(VkPipeline* pipeline)
{
	// Enable dynamic states
	std::vector<VkDynamicState> dynamic_state_enables;
	dynamic_state_enables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	dynamic_state_enables.push_back(VK_DYNAMIC_STATE_SCISSOR);
	VkPipelineDynamicStateCreateInfo dynamic_state = {};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.pDynamicStates = dynamic_state_enables.data();
	dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_state_enables.size());

	// Vertex input binding
	VkVertexInputBindingDescription vertex_input_binding = {};
	vertex_input_binding.binding = 0;
	vertex_input_binding.stride = sizeof(Vertex);
	vertex_input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	// Inpute attribute bindings describe shader attribute locations and memory layouts
	std::array<VkVertexInputAttributeDescription, 2> vertex_input_attributes;
	// These match the following shader layout (see triangle.vert):
	//	layout (location = 0) in vec3 inPos;
	//	layout (location = 1) in vec3 inColor;
	// Attribute location 0: Position
	vertex_input_attributes[0].binding = 0;
	vertex_input_attributes[0].location = 0;
	// Position attribute is three 32 bit signed (SFLOAT) floats (R32 G32 B32)
	vertex_input_attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertex_input_attributes[0].offset = offsetof(Vertex, position);
	// Attribute location 1: Color
	vertex_input_attributes[1].binding = 0;
	vertex_input_attributes[1].location = 1;
	// Color attribute is three 32 bit signed (SFLOAT) floats (R32 G32 B32)
	vertex_input_attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertex_input_attributes[1].offset = offsetof(Vertex, color);

	// Vertex input state used for pipeline creation
	VkPipelineVertexInputStateCreateInfo vertex_input_state = {};
	vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_state.vertexBindingDescriptionCount = 1;
	vertex_input_state.pVertexBindingDescriptions = &vertex_input_binding;
	vertex_input_state.vertexAttributeDescriptionCount = 2;
	vertex_input_state.pVertexAttributeDescriptions = vertex_input_attributes.data();

	// Shaders
	std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages{};

	shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[0].pSpecializationInfo = nullptr;
	shader_stages[0].flags = 0;
	shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shader_stages[0].pName = "main";
	shader_stages[0].module = shader_loader.load(device, "D:\\Documents\\Vulkan\\Projects\\vulkan-renderer\\Data\\shaders\\simple.vert.spv");

	shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[1].pSpecializationInfo = nullptr;
	shader_stages[1].flags = 0;
	shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shader_stages[1].pName = "main";
	shader_stages[1].module = shader_loader.load(device, "D:\\Documents\\Vulkan\\Projects\\vulkan-renderer\\Data\\shaders\\simple.frag.spv");

	// Input assembly state describes how primitives are assembled
	VkPipelineInputAssemblyStateCreateInfo input_assembly;
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// Rasterization state
	VkPipelineRasterizationStateCreateInfo rasterization_state = {};
	rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization_state.cullMode = VK_CULL_MODE_NONE;
	rasterization_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization_state.depthClampEnable = VK_FALSE;
	rasterization_state.rasterizerDiscardEnable = VK_FALSE;
	rasterization_state.depthBiasEnable = VK_FALSE;
	rasterization_state.depthBiasConstantFactor = 0.0f;
	rasterization_state.depthBiasClamp = 0.0f;
	rasterization_state.depthBiasSlopeFactor = 0.0f;
	rasterization_state.lineWidth = 1.0f;

	// Color blend state describes
	VkPipelineColorBlendAttachmentState blend_attachment_state[1] = {};
	blend_attachment_state[0].colorWriteMask = 0xf;
	blend_attachment_state[0].blendEnable = VK_FALSE;
	VkPipelineColorBlendStateCreateInfo color_blend_state = {};
	color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_state.attachmentCount = 1;
	color_blend_state.pAttachments = blend_attachment_state;

	// Viewport state sets the number of viewports and scissor used in this pipeline
	// Note: This is actually overriden by the dynamic states (see below)
	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.scissorCount = 1;

	// Depth and stencil state containing depth and stencil compare and test operations
	// We only use depth tests and want depth tests and writes to be enabled and compare with less or equal
	VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {};
	depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_state.depthTestEnable = VK_TRUE;
	depth_stencil_state.depthWriteEnable = VK_TRUE;
	depth_stencil_state.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
	depth_stencil_state.back.failOp = VK_STENCIL_OP_KEEP;
	depth_stencil_state.back.passOp = VK_STENCIL_OP_KEEP;
	depth_stencil_state.back.compareOp = VK_COMPARE_OP_ALWAYS;
	depth_stencil_state.stencilTestEnable = VK_FALSE;
	depth_stencil_state.front = depth_stencil_state.back;

	// Multi sampling state
	// This example does not make use fo multi sampling (for anti-aliasing), the state must still be set and passed to the pipeline
	VkPipelineMultisampleStateCreateInfo multisample_state = {};
	multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_state.pSampleMask = nullptr;
	multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisample_state.sampleShadingEnable = VK_FALSE;
	multisample_state.alphaToCoverageEnable = VK_FALSE;
	multisample_state.alphaToOneEnable = VK_FALSE;
	multisample_state.minSampleShading = 0.0;

	// Pipeline
	VkGraphicsPipelineCreateInfo pipeline_create_info = {};
	pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_create_info.layout = pipeline_layout;
	pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_create_info.basePipelineIndex = 0;
	pipeline_create_info.pVertexInputState = &vertex_input_state;
	pipeline_create_info.pInputAssemblyState = &input_assembly;
	pipeline_create_info.pRasterizationState = &rasterization_state;
	pipeline_create_info.pColorBlendState = &color_blend_state;
	pipeline_create_info.pTessellationState = nullptr;
	pipeline_create_info.pMultisampleState = &multisample_state;
	pipeline_create_info.pDynamicState = &dynamic_state;
	pipeline_create_info.pViewportState = &viewport_state;
	pipeline_create_info.pDepthStencilState = &depth_stencil_state;
	pipeline_create_info.pStages = shader_stages.data();
	pipeline_create_info.stageCount = static_cast<uint32_t>(shader_stages.size());
	pipeline_create_info.renderPass = render_pass;
	pipeline_create_info.subpass = 0;

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipeline_cache, 1, &pipeline_create_info, nullptr, pipeline));

	vkDestroyShaderModule(device, shader_stages[0].module, nullptr);
	vkDestroyShaderModule(device, shader_stages[1].module, nullptr);
}

void VulkanRenderer::create_semaphores()
{
	VkSemaphoreCreateInfo semaphore_create_info = {};
	semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &image_acquired_semaphore));
	VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &render_complete_semaphore));

	VkFenceCreateInfo fence_create_info = {};
	fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	// Create in signaled state so we don't wait on first render of each command buffer
	fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	draw_fences.resize(command_buffers.size());
	for (auto& fence : draw_fences)
	{
		VK_CHECK_RESULT(vkCreateFence(device, &fence_create_info, nullptr, &fence));
	}
}

void VulkanRenderer::create_command_buffer(const uint32_t &width, const uint32_t &height, std::vector<VkCommandBuffer>& command_buffers)
{
	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	// Set clear values for all framebuffer attachments with loadOp set to clear
	// We use two attachments (color and depth) that are cleared at the start of the subpass and as such we need to set clear values for both
	VkClearValue clearValues[2];
	clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = render_pass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = width;
	renderPassBeginInfo.renderArea.extent.height = height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	for (int32_t i = 0; i < command_buffers.size(); ++i)
	{
		// Set target frame buffer
		renderPassBeginInfo.framebuffer = frame_buffers[i];

		VK_CHECK_RESULT(vkBeginCommandBuffer(command_buffers[i], &cmdBufInfo));

		// Start the first sub pass specified in our default render pass setup by the base class
		// This will clear the color and depth attachment
		vkCmdBeginRenderPass(command_buffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Update dynamic viewport state
		VkViewport viewport = {};
		viewport.height = (float)height;
		viewport.width = (float)width;
		viewport.minDepth = (float) 0.0f;
		viewport.maxDepth = (float) 1.0f;
		vkCmdSetViewport(command_buffers[i], 0, 1, &viewport);

		// Update dynamic scissor state
		VkRect2D scissor = {};
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(command_buffers[i], 0, 1, &scissor);

		// Bind descriptor sets describing shader binding points
		vkCmdBindDescriptorSets(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);

		// Bind the rendering pipeline
		// The pipeline (state object) contains all states of the rendering pipeline, binding it will set all the states specified at pipeline creation time
		vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

		// Bind triangle vertex buffer (contains position and colors)
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(command_buffers[i], 0, 1, &vertex_buffer.buffer, offsets);

		// Bind triangle index buffer
		vkCmdBindIndexBuffer(command_buffers[i], index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		// Draw indexed triangle
		vkCmdDrawIndexed(command_buffers[i], index_buffer.count, 1, 0, 0, 1);

		vkCmdEndRenderPass(command_buffers[i]);

		// Ending the render pass will add an implicit barrier transitioning the frame buffer color attachment to 
		// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for presenting it to the windowing system
		VK_CHECK_RESULT(vkEndCommandBuffer(command_buffers[i]));
	}
}

uint32_t VulkanRenderer::get_memory_type_index(uint32_t type_bits, VkMemoryPropertyFlags properties)
{
	uint32_t memory_type_index;
	device.get_memory_type(type_bits, properties, &memory_type_index);
	return memory_type_index;
}

bool VulkanRenderer::begin_command_buffer(
	VkCommandBuffer command_buffer,
	VkCommandBufferUsageFlags usage = 0,
	VkCommandBufferInheritanceInfo * secondary_command_buffer_info = nullptr)
{
	VkCommandBufferBeginInfo command_buffer_begin_info = {};
	command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_info.flags = usage;
	command_buffer_begin_info.pInheritanceInfo = secondary_command_buffer_info;

	VkResult result = vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);
	if (VK_SUCCESS != result) {
		std::cout << "Could not begin command buffer recording operation." << std::endl;
		return false;
	}
	return true;
}

bool VulkanRenderer::end_command_buffer(VkCommandBuffer command_buffer)
{
	VkResult result = vkEndCommandBuffer(command_buffer);
	if (VK_SUCCESS != result) {
		std::cout << "Error occurred during command buffer recording." << std::endl;
		return false;
	}
	return true;
}