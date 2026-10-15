#include "App.h"

#include <Render/System/SimpleSystem.hpp>
#include <Render/Helpers/Buffer.hpp>

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

	struct GloabalUbo {
		glm::mat4 projection_view{ 1.f };
		glm::vec3 light_direction = glm::normalize(glm::vec3{ 1.f, -3.f, -1.f });
	};

	App::App()
	{
		global_pool = VulkanRender::DescriptorPool::Builder(vulkan_render_device)
			.SetMaxSets(VulkanRender::VulkanRenderSwapChain::MAX_FRAMES_IN_FLIGHT)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VulkanRender::VulkanRenderSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();

		LoadGameObjects();
	}

	App::~App()
	{
	}

	void App::run()
	{
		std::vector<std::unique_ptr<VulkanRender::Buffer>> ubo_buffers(VulkanRender::VulkanRenderSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < ubo_buffers.size(); i++) {
			ubo_buffers[i] = std::make_unique<VulkanRender::Buffer>(
				vulkan_render_device,
				sizeof(GloabalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);

			ubo_buffers[i]->Map();
		}

		auto global_set_layout = VulkanRender::DescriptorSetLayout::Builder(vulkan_render_device)
			.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.Build();

		std::vector<VkDescriptorSet> global_descriptor_sets(VulkanRender::VulkanRenderSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < global_descriptor_sets.size(); i++) {
			auto buffer_info = ubo_buffers[i]->GetDescriptorInfo();
			VulkanRender::DescriptorWriter(*global_set_layout, *global_pool)
				.WriteBuffer(0, &buffer_info)
				.Build(global_descriptor_sets[i]);
		}

        Camera camera{};
        auto view_object = Object::GameObject::CreateGameObject();
        Input::Keyboard keyboard{};

		VulkanRender::SimpleSystem simple_render_system{vulkan_render_device, vulkan_renderer.GetSwapChainRenderPass(), global_set_layout->GetDescriptorSetLayout()};
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
				int frame_index = vulkan_renderer.GetFrameIndex();
				VulkanRender::FrameInfo frame_info{
					frame_index,
					frame_time,
					command_buffer,
					camera,
					global_descriptor_sets[frame_index]
				};
				//Update ubo
				GloabalUbo ubo{};
				ubo.projection_view = camera.GetProjection() * camera.GetView();
				ubo_buffers[frame_index]->WriteToBuffer(&ubo);
				ubo_buffers[frame_index]->Flush();

				//Render
				vulkan_renderer.BeginSwapChainRenderPass(command_buffer);
				simple_render_system.RenderGameObjects(frame_info, game_objects);
				vulkan_renderer.EndSwapChainRenderPass(command_buffer);
				vulkan_renderer.EndFrame();
			}
		}

		vkDeviceWaitIdle(vulkan_render_device.GetDevice());
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