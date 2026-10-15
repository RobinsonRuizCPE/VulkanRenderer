#pragma once

#include <Render/VulkanRenderDevice.hpp>

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace VulkanRender {

    class DescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(VulkanRenderDevice& device) : device{ device } {}

            Builder& AddBinding(
                uint32_t binding,
                VkDescriptorType descriptor_type,
                VkShaderStageFlags stage_flags,
                uint32_t count = 1);
            std::unique_ptr<DescriptorSetLayout> Build() const;

        private:
            VulkanRenderDevice& device;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        DescriptorSetLayout(
            VulkanRenderDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~DescriptorSetLayout();
        DescriptorSetLayout(const DescriptorSetLayout&) = delete;
        DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

        VkDescriptorSetLayout GetDescriptorSetLayout() const { return descriptorSet_layout; }

    private:
        VulkanRenderDevice& vulkan_device;
        VkDescriptorSetLayout descriptorSet_layout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class DescriptorWriter;
    };

    class DescriptorPool {
    public:
        class Builder {
        public:
            Builder(VulkanRenderDevice& device) : vulkan_device{ device } {}

            Builder& AddPoolSize(VkDescriptorType descriptor_type, uint32_t count);
            Builder& SetPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& SetMaxSets(uint32_t count);
            std::unique_ptr<DescriptorPool> build() const;

        private:
            VulkanRenderDevice& vulkan_device;
            std::vector<VkDescriptorPoolSize> pool_sizes{};
            uint32_t max_sets = 1000;
            VkDescriptorPoolCreateFlags pool_flags = 0;
        };

        DescriptorPool(
            VulkanRenderDevice& device,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags pool_flags,
            const std::vector<VkDescriptorPoolSize>& pool_sizes);
        ~DescriptorPool();
        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool& operator=(const DescriptorPool&) = delete;

        bool AllocateDescriptor(const VkDescriptorSetLayout descriptorSet_layout, VkDescriptorSet& descriptor) const;

        void FreeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void ResetPool();

    private:
        VulkanRenderDevice& vulkan_device;
        VkDescriptorPool descriptor_pool;

        friend class DescriptorWriter;
    };

    class DescriptorWriter {
    public:
        DescriptorWriter(DescriptorSetLayout& set_layout, DescriptorPool& pool);

        DescriptorWriter& WriteBuffer(uint32_t binding, VkDescriptorBufferInfo* buffer_info);
        DescriptorWriter& WriteImage(uint32_t binding, VkDescriptorImageInfo* image_info);

        bool Build(VkDescriptorSet& set);
        void Overwrite(VkDescriptorSet& set);

    private:
        DescriptorSetLayout& set_layout;
        DescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}  // namespace lve