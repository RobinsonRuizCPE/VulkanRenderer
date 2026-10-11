#pragma once

#include "VulkanWindow.hpp"
#include "Render/VulkanRenderPipeline.hpp"
#include "Render/VulkanrenderSwapChain.hpp"

#include <memory>
#include <vector>

namespace VWindow {

	class App {
	public:

		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 800;

		App();
		~App();

		App(const App&) = delete;
		App& operator=(const App&) = delete;

		void run();

	private:
		void CreatePipelineLayout();
		void CreatePipeline();
		void CreateCommandBuffers();
		void DrawFrame();

		VulkanWindow vulkan_window{ WIDTH, HEIGHT , "Test App"};

		VulkanRender::VulkanRenderDevice vulkan_render_device{ vulkan_window };
		VulkanRender::VulkanRenderSwapChain vulkan_swap_chain{ vulkan_render_device , vulkan_window.GetExtent() };
		std::unique_ptr<VulkanRender::VulkanRenderPipeline> vulkan_render_pipeline;
		VkPipelineLayout pipeline_layout;
		std::vector<VkCommandBuffer> command_buffers;
	};
}