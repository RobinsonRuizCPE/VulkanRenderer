#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace VWindow {

	class VulkanWindow {
	public:
		VulkanWindow(int width, int height, std::string name);
		~VulkanWindow();

		VulkanWindow(const VulkanWindow&) = delete;
		VulkanWindow& operator=(const VulkanWindow&) = delete;

		bool ShouldClose() { return glfwWindowShouldClose(window); }
		VkExtent2D GetExtent() { return { static_cast<uint32_t>(width) ,  static_cast<uint32_t>(height) }; }

		void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

		bool WasWindowResized() const { return frame_buffer_resize; }
		void SetWindowResizeFlag(bool value) { frame_buffer_resize = value; }
		void SetWindowSize(int w, int h) { SetWindowResizeFlag(true); width = w, height = h; }

	private:
		void InitWindow();
		static void FrameBufferResizeCallback(GLFWwindow* windows, int width, int height);

		int width;
		int height;
		bool frame_buffer_resize = false;

		std::string window_name;
		GLFWwindow* window;
	};
}