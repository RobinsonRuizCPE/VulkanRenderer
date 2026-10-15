#pragma once

#include <VulkanWindow.hpp>

// std lib headers
#include <string>
#include <vector>

namespace VulkanRender {

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> present_modes;
};

struct QueueFamilyIndices {
	uint32_t graphics_family;
	uint32_t present_family;
	bool graphics_family_has_value = false;
	bool present_family_has_value = false;
	bool IsComplete() { return graphics_family_has_value && present_family_has_value; }
};

class VulkanRenderDevice {
 public:
#ifdef NDEBUG
	const bool enable_validation_layers = false;
#else
	const bool enable_validation_layers = true;
#endif

	VulkanRenderDevice(VWindow::VulkanWindow &window);
	~VulkanRenderDevice();

	// Not copyable or movable
	VulkanRenderDevice(const VulkanRenderDevice &) = delete;
	VulkanRenderDevice operator=(const VulkanRenderDevice &) = delete;
	VulkanRenderDevice(VulkanRenderDevice &&) = delete;
	VulkanRenderDevice &operator=(VulkanRenderDevice &&) = delete;

	VkCommandPool GetCommandPool() { return command_pool; }
	VkDevice GetDevice() { return logical_device; }
	VkSurfaceKHR GetSurface() { return surface; }
	VkQueue GetGraphicsQueue() { return graphics_queue; }
	VkQueue GetPresentQueue() { return present_queue; }

	SwapChainSupportDetails GetSwapChainSupport() { return QuerySwapChainSupport(physical_device); }
	uint32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);
	QueueFamilyIndices FindPhysicalQueueFamilies() { return FindQueueFamilies(physical_device); }
	VkFormat FindSupportedFormat(
			const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	// Buffer Helper Functions
	void CreateBuffer(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkBuffer &buffer,
			VkDeviceMemory &buffer_memory);
	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer command_buffer);
	void CopyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
	void CopyBufferToImage(
			VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer_count);

	void CreateImageWithInfo(
			const VkImageCreateInfo &image_info,
			VkMemoryPropertyFlags properties,
			VkImage &image,
			VkDeviceMemory &image_memory);

	VkPhysicalDeviceProperties properties;

 private:
	void CreateInstance();
	void SetupDebugMessenger();
	void CreateSurface();
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateCommandPool();

	// helper functions
	bool IsDeviceSuitable(VkPhysicalDevice device);
	std::vector<const char *> GetRequiredExtensions();
	bool CheckValidationLayerSupport();
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &create_info);
	void HasGlfwRequiredInstanceExtensions();
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VWindow::VulkanWindow &window;
	VkCommandPool command_pool;

	VkDevice logical_device;
	VkSurfaceKHR surface;
	VkQueue graphics_queue;
	VkQueue present_queue;

	const std::vector<const char *> validation_layers = {"VK_LAYER_KHRONOS_validation"};
	const std::vector<const char *> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

} 