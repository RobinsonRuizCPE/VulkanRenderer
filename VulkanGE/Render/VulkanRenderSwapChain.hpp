#pragma once

#include "VulkanRenderDevice.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <string>
#include <vector>
#include <memory>

namespace VulkanRender {

	class VulkanRenderSwapChain {
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

		VulkanRenderSwapChain(VulkanRenderDevice& device_ref, VkExtent2D window_extent);
		VulkanRenderSwapChain(VulkanRenderDevice& device_ref, VkExtent2D window_extent, std::shared_ptr< VulkanRenderSwapChain> previous);

		~VulkanRenderSwapChain();

		VulkanRenderSwapChain(const VulkanRenderSwapChain&) = delete;
		VulkanRenderSwapChain operator=(const VulkanRenderSwapChain&) = delete;

		VkFramebuffer GetFrameBuffer(int index) { return swap_chain_framebuffers[index]; }
		VkRenderPass GetRenderPass() { return render_pass; }
		VkImageView GetImageView(int index) { return swap_chain_image_views[index]; }
		size_t GetImageCount() { return swap_chain_images.size(); }
		VkFormat GetSwapChainImageFormat() { return swap_chain_image_format; }
		VkExtent2D GetSwapChainExtent() { return swap_chain_extent; }
		uint32_t GetWidth() { return swap_chain_extent.width; }
		uint32_t GetHeight() { return swap_chain_extent.height; }

		float GetExtentAspectRatio() {
			return static_cast<float>(swap_chain_extent.width) / static_cast<float>(swap_chain_extent.height);
		}
		VkFormat FindDepthFormat();

		VkResult AcquireNextImage(uint32_t* image_index);
		VkResult SubmitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* image_index);
		bool CompareSwapFormats(const VulkanRenderSwapChain& swap_chain) const {
			return swap_chain.swap_chain_depth_format == swap_chain_depth_format &&
				swap_chain.swap_chain_image_format == swap_chain_image_format;
		}

	private:
		void Init();
		void CreateSwapChain();
		void CreateImageViews();
		void CreateDepthResources();
		void CreateRenderPass();
		void CreateFramebuffers();
		void CreateSyncObjects();

		// Helper functions
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
			const std::vector<VkSurfaceFormatKHR>& available_formats);
		VkPresentModeKHR ChooseSwapPresentMode(
			const std::vector<VkPresentModeKHR>& available_present_modes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		std::shared_ptr<VulkanRenderSwapChain> old_swap_chain;

		VkFormat swap_chain_image_format;
		VkFormat swap_chain_depth_format;
		VkExtent2D swap_chain_extent;

		std::vector<VkFramebuffer> swap_chain_framebuffers;
		VkRenderPass render_pass;

		std::vector<VkImage> depth_images;
		std::vector<VkDeviceMemory> depth_image_memories;
		std::vector<VkImageView> depth_image_views;
		std::vector<VkImage> swap_chain_images;
		std::vector<VkImageView> swap_chain_image_views;

		VulkanRenderDevice& device;
		VkExtent2D window_extent;

		VkSwapchainKHR swap_chain;

		std::vector<VkSemaphore> image_available_semaphores;
		std::vector<VkSemaphore> render_finished_semaphores;
		std::vector<VkFence> in_flight_fences;
		std::vector<VkFence> images_in_flight;
		size_t current_frame = 0;
	};

}  // namespace lve
