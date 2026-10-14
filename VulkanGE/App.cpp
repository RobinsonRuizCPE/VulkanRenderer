#include "App.h"

#include <Render/System/SimpleSystem.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <array>

namespace VWindow {

	App::App()
	{
		LoadGameObjects();
	}

	App::~App()
	{
	}

	void App::run()
	{
		VulkanRender::SimpleSystem simple_render_system{vulkan_render_device, vulkan_renderer.GetSwapChainRenderPass()};
		while (!vulkan_window.ShouldClose()) {
			glfwPollEvents(); // Polling for mouse click for closing, minimizing, ....
			if (auto command_buffer = vulkan_renderer.BeginFrame()) {
				vulkan_renderer.BeginSwapChainRenderPass(command_buffer);
				simple_render_system.RenderGameObjects(command_buffer, game_objects);
				vulkan_renderer.EndSwapChainRenderPass(command_buffer);
				vulkan_renderer.EndFrame();
			}
		}

		vkDeviceWaitIdle(vulkan_render_device.device());
	}

	void App::LoadGameObjects()
	{
		std::vector<VulkanRender::Model::Vertex> vertices{
			{{0.0f, -0.5f}, {1,0,0}},
			{{0.5f, 0.5f}, {0,1,0}},
			{{-0.5f, 0.5f}, {0,0,1}}
		};

		auto model = std::make_shared<VulkanRender::Model>(vulkan_render_device, vertices);
		auto triangle = Object::GameObject::CreateGameObject();
		triangle.model = model;
		triangle.color = { 0.1, 0.8, 0.1 };
		triangle.transform_2d.translation.x = 0.2f;
		triangle.transform_2d.scale = { 2.f, 0.5f};
		triangle.transform_2d.rotation = 0.25 * glm::two_pi<float>();
		game_objects.push_back(std::move(triangle));
	}
	
}