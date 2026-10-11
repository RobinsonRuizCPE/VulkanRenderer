#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace VWindow {

	class VulkanWindow{
	public:
		VulkanWindow(int width, int height, std::string name);
		~VulkanWindow();

		VulkanWindow(const VulkanWindow&) = delete;
		VulkanWindow &operator=(const VulkanWindow&) = delete;

		bool ShouldClose() { return glfwWindowShouldClose(window); }
		VkExtent2D GetExtent() { return { static_cast<uint32_t>(width) ,  static_cast<uint32_t>(height) }; }
		void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		void InitWindow();

		const int width;
		const int height;

		std::string window_name;
		GLFWwindow* window;
	};
}