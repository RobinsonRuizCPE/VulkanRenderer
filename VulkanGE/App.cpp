#include "App.h"

#include <stdexcept>
#include <array>

namespace VWindow {

	App::App()
	{
		CreatePipelineLayout();
		CreatePipeline();
		CreateCommandBuffers();
	}

	App::~App()
	{
		vkDestroyPipelineLayout(vulkan_render_device.device(), pipeline_layout, nullptr);
	}

	void App::run()
	{
		while (!vulkan_window.ShouldClose()) {
			glfwPollEvents(); // Polling for mouse click for closing, minimizing, ....
			DrawFrame();
		}

		vkDeviceWaitIdle(vulkan_render_device.device());
	}

	void App::CreatePipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = 0;
		pipeline_layout_info.pSetLayouts = nullptr;
		pipeline_layout_info.pushConstantRangeCount = 0;
		pipeline_layout_info.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(vulkan_render_device.device(), &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create pipeline layout");
		}
	}

	void App::CreatePipeline()
	{
		auto pipeline_config = VulkanRender::VulkanRenderPipeline::DefaultPipelineConfigInfo(vulkan_swap_chain.width(), vulkan_swap_chain.height());
		pipeline_config.render_pass = vulkan_swap_chain.getRenderPass();
		pipeline_config.pipeline_layout = pipeline_layout;
		vulkan_render_pipeline = std::make_unique<VulkanRender::VulkanRenderPipeline>(vulkan_render_device, "Render/Shader/BaseVertexShader.vert.spv", "Render/Shader/BaseFragmentShader.frag.spv", pipeline_config);

	}

	void App::CreateCommandBuffers()
	{
		command_buffers.resize(vulkan_swap_chain.imageCount());
		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = vulkan_render_device.getCommandPool();
		alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());
		if (vkAllocateCommandBuffers(vulkan_render_device.device(), &alloc_info, command_buffers.data())!=VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffer");
		}

		for (int i = 0; i < command_buffers.size(); i++) {
			VkCommandBufferBeginInfo begin_info{};
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			if (vkBeginCommandBuffer(command_buffers[i], &begin_info) != VK_SUCCESS) {
				throw std::runtime_error("Failed to begin recording command buffer");
			}

			VkRenderPassBeginInfo render_pass_info{};
			render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_info.renderPass = vulkan_swap_chain.getRenderPass();
			render_pass_info.framebuffer = vulkan_swap_chain.getFrameBuffer(i);
			render_pass_info.renderArea.offset = { 0, 0 };
			render_pass_info.renderArea.extent = vulkan_swap_chain.getSwapChainExtent();

			std::array<VkClearValue, 2> clear_values{};
			clear_values[0].color = { 0.1f ,0.1f ,0.1f ,1.0f };
			clear_values[1].depthStencil = { 1.0f, 0 };
			render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
			render_pass_info.pClearValues = clear_values.data();

			vkCmdBeginRenderPass(command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

			vulkan_render_pipeline->Bind(command_buffers[i]);
			vkCmdDraw(command_buffers[i], 3, 1, 0, 0);
			vkCmdEndRenderPass(command_buffers[i]);
			if (vkEndCommandBuffer(command_buffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to record command buffer");
			}
		}
	}

	void App::DrawFrame()
	{
		uint32_t image_index;
		auto result = vulkan_swap_chain.acquireNextImage(&image_index);
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to aquire swap chain image");
		}

		result = vulkan_swap_chain.submitCommandBuffers(&command_buffers[image_index], &image_index);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swap chain image");
		}
	}

}