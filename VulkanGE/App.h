#pragma once

#include "VulkanWindow.hpp"
#include "Render/VulkanRenderer.hpp"
#include <Render/Model/Model.hpp>
#include <GameObject/GameObject.hpp>

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
		void LoadGameObjects();

		VulkanWindow vulkan_window{ WIDTH, HEIGHT , "Test App"};

		VulkanRender::VulkanRenderDevice vulkan_render_device{ vulkan_window };
		VulkanRender::Renderer vulkan_renderer{ vulkan_window, vulkan_render_device };
		std::vector<Object::GameObject> game_objects;
	};
}