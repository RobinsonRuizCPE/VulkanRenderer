#pragma once

#include <Render/VulkanRenderSwapChain.hpp>
#include <Render/Model/Model.hpp>

#include <memory>
#include <vector>
#include <cassert>

namespace VulkanRender {

	class Renderer {
	public:

		Renderer(VWindow::VulkanWindow& vulkan_window, VulkanRenderDevice& device);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		VkRenderPass GetSwapChainRenderPass() const { return vulkan_swap_chain->GetRenderPass(); }
		float GetAspectRation()const { return vulkan_swap_chain->GetExtentAspectRatio(); }
		bool IsFrameInProgress() const { return is_frame_stared; }
		VkCommandBuffer GetCurrentCommandBuffer() const { 
			assert(is_frame_stared && "Cannot get command buffer when frame in progress");
			return command_buffers[current_frame_index];
		}

		int GetFrameIndex() const {
			assert(is_frame_stared && "Cannot get frame index when frame in progress");
			return current_frame_index;
		}

		VkCommandBuffer BeginFrame();
		void EndFrame();
		void BeginSwapChainRenderPass(VkCommandBuffer command_buffer);
		void EndSwapChainRenderPass(VkCommandBuffer command_buffer);


	private:
		void CreateCommandBuffers();
		void DrawFrame();
		void RecreateSwapchain();
		void FreeCommandBuffer();

		VWindow::VulkanWindow& vulkan_window;
		VulkanRender::VulkanRenderDevice& vulkan_render_device;
		std::unique_ptr<VulkanRender::VulkanRenderSwapChain> vulkan_swap_chain;
		std::vector<VkCommandBuffer> command_buffers;

		uint32_t current_image_index;
		int current_frame_index = 0;
		bool is_frame_stared = false;
	};
}