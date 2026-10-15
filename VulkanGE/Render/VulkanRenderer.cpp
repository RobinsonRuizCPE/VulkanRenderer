#include "VulkanRenderer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <array>


namespace VulkanRender {

	Renderer::Renderer(VWindow::VulkanWindow& window, VulkanRenderDevice& device) : vulkan_window{window}, vulkan_render_device{device}
	{
		RecreateSwapchain();
		CreateCommandBuffers();
	}

	Renderer::~Renderer()
	{
		FreeCommandBuffer();
	}

	VkCommandBuffer Renderer::BeginFrame()
	{
		assert(!is_frame_stared && "Can't call begin frame while already in progress");
		auto result = vulkan_swap_chain->AcquireNextImage(&current_image_index);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			RecreateSwapchain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to aquire swap chain image");
		}

		is_frame_stared = true;
		auto command_buffer = GetCurrentCommandBuffer();
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording command buffer");
		}

		return command_buffer;
	}

	void Renderer::EndFrame()
	{
		assert(is_frame_stared && "Can't call end frame while not in progress");
		auto command_buffer = GetCurrentCommandBuffer();
		if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer");
		}

		auto result = vulkan_swap_chain->SubmitCommandBuffers(&command_buffer, &current_image_index);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vulkan_window.WasWindowResized()) {
			vulkan_window.SetWindowResizeFlag(false);
			RecreateSwapchain();
		}

		else if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swap chain image");
		}

		is_frame_stared = false;
		current_frame_index = (current_frame_index + 1) % VulkanRenderSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void Renderer::BeginSwapChainRenderPass(VkCommandBuffer command_buffer)
	{
		assert(is_frame_stared && "Can't call BeginSwapChainRenderPass while frame not in progress");
		assert(command_buffer == GetCurrentCommandBuffer() && "Can't call BeginSwapChainRenderPass on a command buffer from a different frame");


		VkRenderPassBeginInfo render_pass_info{};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = vulkan_swap_chain->GetRenderPass();
		render_pass_info.framebuffer = vulkan_swap_chain->GetFrameBuffer(current_image_index);
		render_pass_info.renderArea.offset = { 0, 0 };
		render_pass_info.renderArea.extent = vulkan_swap_chain->GetSwapChainExtent();

		std::array<VkClearValue, 2> clear_values{};
		clear_values[0].color = { 0.01f ,0.01f ,0.01f ,1.0f };
		clear_values[1].depthStencil = { 1.0f, 0 };
		render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(vulkan_swap_chain->GetSwapChainExtent().width);
		viewport.height = static_cast<float>(vulkan_swap_chain->GetSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, vulkan_swap_chain->GetSwapChainExtent() };
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);
		vkCmdSetScissor(command_buffer, 0, 1, &scissor);
	}

	void Renderer::EndSwapChainRenderPass(VkCommandBuffer command_buffer)
	{
		assert(is_frame_stared && "Can't call EndSwapChainRenderPass while frame not in progress");
		assert(command_buffer == GetCurrentCommandBuffer() && "Can't call EndSwapChainRenderPass on a command buffer from a different frame");
		vkCmdEndRenderPass(command_buffer);

	}

	void Renderer::CreateCommandBuffers()
	{
		command_buffers.resize(VulkanRenderSwapChain::MAX_FRAMES_IN_FLIGHT);
		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = vulkan_render_device.GetCommandPool();
		alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());
		if (vkAllocateCommandBuffers(vulkan_render_device.GetDevice(), &alloc_info, command_buffers.data())!=VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffer");
		}
	}

	void Renderer::RecreateSwapchain()
	{
		auto extent = vulkan_window.GetExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = vulkan_window.GetExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(vulkan_render_device.GetDevice());
		if (vulkan_swap_chain == nullptr) {
			vulkan_swap_chain = std::make_unique<VulkanRender::VulkanRenderSwapChain>(vulkan_render_device, extent);
		}
		else {
			std::shared_ptr<VulkanRenderSwapChain> old_swapchain = std::move(vulkan_swap_chain);
			vulkan_swap_chain = std::make_unique<VulkanRender::VulkanRenderSwapChain>(vulkan_render_device, extent, old_swapchain);
			if (!old_swapchain->CompareSwapFormats(*vulkan_swap_chain.get())) {
				throw std::runtime_error("Swap chain image/depth format has changed");
			}
		}
	}

	void Renderer::FreeCommandBuffer()
	{
		vkFreeCommandBuffers(vulkan_render_device.GetDevice(), vulkan_render_device.GetCommandPool(), static_cast<uint32_t>(command_buffers.size()), command_buffers.data());
		command_buffers.clear();
	}


}