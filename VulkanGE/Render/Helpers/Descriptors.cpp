#include "Descriptors.hpp"

// std
#include <cassert>
#include <stdexcept>

namespace VulkanRender {

    // *************** Descriptor Set Layout Builder *********************

    DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::AddBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count) {
        assert(bindings.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        bindings[binding] = layoutBinding;
        return *this;
    }

    std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::Build() const {
        return std::make_unique<DescriptorSetLayout>(device, bindings);
    }

    // *************** Descriptor Set Layout *********************

    DescriptorSetLayout::DescriptorSetLayout(
        VulkanRenderDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : vulkan_device{ device }, bindings{ bindings } {
        std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings{};
        for (auto& kv : bindings) {
            set_layout_bindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSet_layout_info{};
        descriptorSet_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSet_layout_info.bindingCount = static_cast<uint32_t>(set_layout_bindings.size());
        descriptorSet_layout_info.pBindings = set_layout_bindings.data();

        if (vkCreateDescriptorSetLayout(
            vulkan_device.GetDevice(),
            &descriptorSet_layout_info,
            nullptr,
            &descriptorSet_layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    DescriptorSetLayout::~DescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(vulkan_device.GetDevice(), descriptorSet_layout, nullptr);
    }

    // *************** Descriptor Pool Builder *********************

    DescriptorPool::Builder& DescriptorPool::Builder::AddPoolSize(
        VkDescriptorType descriptorType, uint32_t count) {
        pool_sizes.push_back({ descriptorType, count });
        return *this;
    }

    DescriptorPool::Builder& DescriptorPool::Builder::SetPoolFlags(
        VkDescriptorPoolCreateFlags flags) {
        pool_flags = flags;
        return *this;
    }
    DescriptorPool::Builder& DescriptorPool::Builder::SetMaxSets(uint32_t count) {
        max_sets = count;
        return *this;
    }

    std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const {
        return std::make_unique<DescriptorPool>(vulkan_device, max_sets, pool_flags, pool_sizes);
    }

    // *************** Descriptor Pool *********************

    DescriptorPool::DescriptorPool(
        VulkanRenderDevice& device,
        uint32_t max_sets,
        VkDescriptorPoolCreateFlags pool_flags,
        const std::vector<VkDescriptorPoolSize>& pool_sizes)
        : vulkan_device{ device } {
        VkDescriptorPoolCreateInfo descriptor_pool_info{};
        descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        descriptor_pool_info.pPoolSizes = pool_sizes.data();
        descriptor_pool_info.maxSets = max_sets;
        descriptor_pool_info.flags = pool_flags;

        if (vkCreateDescriptorPool(vulkan_device.GetDevice(), &descriptor_pool_info, nullptr, &descriptor_pool) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    DescriptorPool::~DescriptorPool() {
        vkDestroyDescriptorPool(vulkan_device.GetDevice(), descriptor_pool, nullptr);
    }

    bool DescriptorPool::AllocateDescriptor(
        const VkDescriptorSetLayout descriptorSet_layout, VkDescriptorSet& descriptor) const {
        VkDescriptorSetAllocateInfo alloc_nfo{};
        alloc_nfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_nfo.descriptorPool = descriptor_pool;
        alloc_nfo.pSetLayouts = &descriptorSet_layout;
        alloc_nfo.descriptorSetCount = 1;

        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenever an old pool fills up. But this is beyond our current scope
        if (vkAllocateDescriptorSets(vulkan_device.GetDevice(), &alloc_nfo, &descriptor) != VK_SUCCESS) {
            return false;
        }
        return true;
    }

    void DescriptorPool::FreeDescriptors(std::vector<VkDescriptorSet>& descriptors) const {
        vkFreeDescriptorSets(
            vulkan_device.GetDevice(),
            descriptor_pool,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
    }

    void DescriptorPool::ResetPool() {
        vkResetDescriptorPool(vulkan_device.GetDevice(), descriptor_pool, 0);
    }

    // *************** Descriptor Writer *********************

    DescriptorWriter::DescriptorWriter(DescriptorSetLayout& set_layout, DescriptorPool& pool)
        : set_layout{ set_layout }, pool{ pool } {
    }

    DescriptorWriter& DescriptorWriter::WriteBuffer(
        uint32_t binding, VkDescriptorBufferInfo* buffer_info) {
        assert(set_layout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = set_layout.bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = buffer_info;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    DescriptorWriter& DescriptorWriter::WriteImage(
        uint32_t binding, VkDescriptorImageInfo* imageInfo) {
        assert(set_layout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = set_layout.bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    bool DescriptorWriter::Build(VkDescriptorSet& set) {
        bool success = pool.AllocateDescriptor(set_layout.GetDescriptorSetLayout(), set);
        if (!success) {
            return false;
        }
        Overwrite(set);
        return true;
    }

    void DescriptorWriter::Overwrite(VkDescriptorSet& set) {
        for (auto& write : writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool.vulkan_device.GetDevice(), writes.size(), writes.data(), 0, nullptr);
    }

}  // namespace lve
