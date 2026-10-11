#include "VulkanWindow.hpp"

#include <stdexcept>

namespace VWindow {

	VulkanWindow::VulkanWindow(int width, int height, std::string name) : width(width), height(height),window_name(name)
	{
		InitWindow();
	}

	VulkanWindow::~VulkanWindow()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void VulkanWindow::InitWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Not OpenGL Used
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // No resize

		window = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);
	}

	void VulkanWindow::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface");
		}
	}

}