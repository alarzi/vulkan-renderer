#pragma once

#include <cassert>
#include <iostream>
#include <array>
#include <vector>

#include <Windows.h>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanPresentationSurface.h"
#include "VulkanShader.h"

#include "VulkanTools.h"
#include "../Framework/Properties.h"

struct DepthBuffer {
	VkFormat format;
	VkImage image;
	VkImageView view;
	VkDeviceMemory memory;
};

struct VulkanBuffer {
	VkBuffer buffer;
	VkDeviceMemory memory;
	VkDescriptorBufferInfo buffer_info;
};

struct VulkanIndexBuffer {
	VkBuffer buffer;
	VkDeviceMemory memory;
	uint32_t count;
};

struct Vertex {
	float position[3];
	float color[3];
};

struct ModelViewProjectMatrix {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
};

class VULKAN_RENDERER_API VulkanRenderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();

	bool							initialize_(int hWnd, int width, int height);
	bool							initialize(HINSTANCE hInstance, HWND hWnd, uint32_t width, uint32_t height);
	void							resize(uint32_t width, uint32_t height);
	void							render();
	void							shutdown();

	//static VulkanRenderer*			get_instance();

private:

	static VulkanRenderer*			_instance;

	/** @brief Instance */
	VulkanInstance					instance;
	/** @brief Device */
	VulkanDevice					device;
	/** @brief Swapchain */
	VulkanSwapchain					swapchain;
	/** @brief PresentationSurface */
	VulkanPresentationSurface		presentation_surface;
	/* @brief VulkanShader */
	VulkanShader					shader_loader;

	/* buffers */
	VkCommandPool					command_pool;
	std::vector<VkCommandBuffer>	command_buffers;

	std::vector<VkFramebuffer>		frame_buffers;
	
	DepthBuffer						depth_buffer;
	VulkanBuffer					uniform_buffer;
	VulkanBuffer					vertex_buffer;
	VulkanIndexBuffer				index_buffer;;
	uint32_t						current_buffer_index = 0;

	VkRenderPass					render_pass;
	VkSemaphore						image_acquired_semaphore;
	VkSemaphore						render_complete_semaphore;
	std::vector<VkFence>			draw_fences;

	/* pipeline */
	VkPipelineLayout				pipeline_layout;
	VkDescriptorPool				descriptor_pool;
	VkDescriptorSet					descriptor_set;
	VkDescriptorSetLayout			descriptor_set_layout;
	
	VkPipeline						graphics_pipeline;
	VkPipelineCache					pipeline_cache;

	ModelViewProjectMatrix			mvp_matrix;

	bool							is_ready;

	bool create_depth_buffer(const uint32_t width, const uint32_t height, DepthBuffer* depth_buffer);

	bool create_buffer(VkDevice logical_device, VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer & buffer);
	bool create_command_pool(VkDevice logical_device, VkCommandPoolCreateFlags parameters, uint32_t queue_family, VkCommandPool* command_pool);
	bool allocate_command_buffer(VkDevice logical_device, VkCommandPool command_pool, VkCommandBufferLevel level, uint32_t count, std::vector<VkCommandBuffer> & command_buffers);
	
	bool create_descriptor_set_layout(VkDescriptorSetLayout* descriptor_set_layout);
	void create_pipeline_layout(VkPipelineLayout* pipeline_layout);

	void create_render_pass(VkRenderPass* render_pass);
	void create_frame_buffer(const uint32_t &width, const uint32_t &height, std::vector<VkFramebuffer> & frame_buffers);

	void create_uniform_buffer(VulkanBuffer* uniform_buffer);
	void update_uniform_buffer(const uint32_t &width, const uint32_t &height, VulkanBuffer* uniform_buffer);
	void create_vertex_buffer(VulkanBuffer* vertex_buffer, VulkanIndexBuffer* index_buffer);

	bool create_descriptor_pool(VkDescriptorPool *descriptor_pool);
	void create_descriptor_set(VkDescriptorSet* descriptor_set);

	void create_pipeline_cache(VkPipelineCache* pipeline_cache);
	void create_graphics_pipeline(VkPipeline* pipeline);

	void create_command_buffer(const uint32_t &width, const uint32_t &height, std::vector<VkCommandBuffer>&command_buffers);

	void create_semaphores();
	uint32_t get_memory_type_index(uint32_t type_bits, VkMemoryPropertyFlags properties);

	
	
	














	
	
	bool begin_command_buffer(VkCommandBuffer command_buffer, VkCommandBufferUsageFlags usage, VkCommandBufferInheritanceInfo * secondary_command_buffer_info);
	bool end_command_buffer(VkCommandBuffer command_buffer);
	bool reset_command_buffer(VkCommandBuffer command_buffer, bool release_resources);
	bool reset_command_pool(VkDevice logical_device, VkCommandPool command_pool, bool release_resources);
	bool create_semaphore(VkDevice logical_device, VkSemaphore & semaphore);
	bool create_fence(VkDevice logical_device, bool signaled, VkFence  & fence);
	bool wait_for_fences(VkDevice logical_device, std::vector<VkFence> const & fences, VkBool32 wait_for_all, uint64_t timeout);
	bool reset_fences(VkDevice logical_device, std::vector<VkFence> const & fences);
	//bool submit_command_buffers_to_queue(VkQueue queue, std::vector<WaitSemaphoreInfo> wait_semaphore_infos, std::vector<VkCommandBuffer> command_buffers, std::vector<VkSemaphore> signal_semaphores, VkFence fence);
	bool wait_for_all_submitted_commands_to_be_finished(VkDevice logical_device);

	
	bool allocate_and_bind_memory_object_to_buffer(VkPhysicalDevice physical_device, VkDevice logical_device, VkBuffer buffer, VkMemoryPropertyFlagBits memory_properties, VkDeviceMemory & memory_object);
	//void set_buffer_memory_barrier(VkCommandBuffer command_buffer, VkPipelineStageFlags generating_stages, VkPipelineStageFlags consuming_stages, std::vector<BufferTransition> buffer_transitions);
	bool create_buffer_view(VkDevice logical_device, VkBuffer buffer, VkFormat format, VkDeviceSize memory_offset, VkDeviceSize memory_range, VkBufferView & buffer_view);
	bool create_image(VkDevice logical_device, VkImageType type, VkFormat format, VkExtent3D size, uint32_t num_mipmaps, uint32_t num_layers, VkSampleCountFlagBits samples, VkImageUsageFlags usage_scenarios, bool cubemap, VkImage & image);
	//void set_image_memory_barrier(VkCommandBuffer command_buffer,VkPipelineStageFlags generating_stages,VkPipelineStageFlags consuming_stages,std::vector<ImageTransition> image_transitions);
	
};