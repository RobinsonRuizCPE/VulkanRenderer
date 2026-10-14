#include "App.h"

#include <Render/System/SimpleSystem.hpp>
#include <Camera/Camera.hpp>
#include <Input/Keyboard.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <array>
#include <chrono>

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
        Camera camera{};
        auto view_object = Object::GameObject::CreateGameObject();
        Input::Keyboard keyboard{};

		VulkanRender::SimpleSystem simple_render_system{vulkan_render_device, vulkan_renderer.GetSwapChainRenderPass()};
        auto current_time = std::chrono::high_resolution_clock::now();
		while (!vulkan_window.ShouldClose()) {
			glfwPollEvents(); // Polling for mouse click for closing, minimizing, ....

            auto new_time = std::chrono::high_resolution_clock::now();
            float frame_time = std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time).count();
            current_time = new_time;

            keyboard.MoveInPlaneXZ(vulkan_window.GetGLFWWindow(), frame_time, view_object);
            camera.SetViewYXZ(view_object.transform.translation, view_object.transform.rotation);
            float aspect = vulkan_renderer.GetAspectRation();
            // camera.SetOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
            camera.SetPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);
			if (auto command_buffer = vulkan_renderer.BeginFrame()) {
				vulkan_renderer.BeginSwapChainRenderPass(command_buffer);
				simple_render_system.RenderGameObjects(command_buffer, game_objects, camera);
				vulkan_renderer.EndSwapChainRenderPass(command_buffer);
				vulkan_renderer.EndFrame();
			}
		}

		vkDeviceWaitIdle(vulkan_render_device.device());
	}


	void App::LoadGameObjects()
	{
		
        std::shared_ptr<VulkanRender::Model> model = VulkanRender::Model::CreateModelfromFile(vulkan_render_device, "Data/Model3D/animated-bee/source/Bee_fbx.glb");
        auto cube = Object::GameObject::CreateGameObject();
        cube.model = model;
        cube.transform.translation = { 0,0,2.5 };
        cube.transform.scale = { 0.1,0.1,0.1 };
        game_objects.push_back(std::move(cube));

	}
	
}