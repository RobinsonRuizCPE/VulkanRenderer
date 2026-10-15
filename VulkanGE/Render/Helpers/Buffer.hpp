#pragma once

#include <Render/VulkanRenderDevice.hpp>

namespace VulkanRender {

	class Buffer {
	public:
		Buffer(
			VulkanRenderDevice& device,
			VkDeviceSize instance_size,
			uint32_t instance_count,
			VkBufferUsageFlags usage_flags,
			VkMemoryPropertyFlags memory_property_flags,
			VkDeviceSize min_offset_alignment = 1);
		~Buffer();

		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;

		VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void Unmap();

		void WriteToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkDescriptorBufferInfo GetDescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

		void WriteToIndex(void* data, int index);
		VkResult FlushIndex(int index);
		VkDescriptorBufferInfo GetDescriptorInfoForIndex(int index);
		VkResult InvalidateIndex(int index);

		VkBuffer GetBuffer() const { return buffer; }
		void* GetMappedMemory() const { return mapped; }
		uint32_t GetInstanceCount() const { return instance_count; }
		VkDeviceSize GetInstanceSize() const { return instance_size; }
		VkDeviceSize GetAlignmentSize() const { return instance_size; }
		VkBufferUsageFlags GetUsageFlags() const { return usage_flags; }
		VkMemoryPropertyFlags GetMemoryPropertyFlags() const { return memory_property_flags; }
		VkDeviceSize GetBufferSize() const { return buffer_size; }

	private:
		static VkDeviceSize GetAlignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment);

		VulkanRenderDevice& device;
		void* mapped = nullptr;
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;

		VkDeviceSize buffer_size;
		uint32_t instance_count;
		VkDeviceSize instance_size;
		VkDeviceSize alignment_size;
		VkBufferUsageFlags usage_flags;
		VkMemoryPropertyFlags memory_property_flags;
	};

}  // namespace lve