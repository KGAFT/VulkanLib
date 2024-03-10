//
// Created by kgaft on 11/19/23.
//

#ifndef VULKANLIB_BUFFER_HPP
#define VULKANLIB_BUFFER_HPP

#include <vulkan/vulkan.hpp>
#include "VulkanLib/MemoryUtils/IDestroyableObject.hpp"
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "VulkanLib/MemoryUtils/SeriesObject.hpp"
#include <memory>

class Buffer : public IDestroyableObject {
private:
    static inline auto requirements = SeriesObject<vk::MemoryRequirements>();
    static inline auto allocInfos = SeriesObject<vk::MemoryAllocateInfo>();
    static inline auto flagsInfo = SeriesObject<vk::MemoryAllocateFlagsInfo>();
    static inline auto createInfos = SeriesObject<vk::BufferCreateInfo>();
public:
    Buffer(std::shared_ptr<LogicalDevice> device, vk::BufferCreateInfo *createInfo,
           vk::MemoryPropertyFlags memoryFlags);

    Buffer(std::shared_ptr<LogicalDevice> device, size_t size, vk::BufferUsageFlags usageFlags,
           vk::MemoryPropertyFlags memoryFlags);

    explicit Buffer(const std::shared_ptr<LogicalDevice> &device);

private:
    vk::Buffer buffer = nullptr;
    vk::DeviceMemory bufferMemory = nullptr;
    std::shared_ptr<LogicalDevice> device;
    SeriesObject<vk::BufferCopy> copyRegions;
    size_t bufferSize;
    vk::BufferDeviceAddressInfo addressInfo{};
public:
    void initialize(size_t size, vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryFlags);

    void initialize(vk::BufferCreateInfo *createInfo, vk::MemoryPropertyFlags memoryFlags);

    void copyFromBuffer(vk::CommandBuffer cmd, Buffer &source, size_t size, size_t srcOffset, size_t dstOffset);

    void copyToBuffer(vk::CommandBuffer cmd, Buffer &destination, size_t size, size_t srcOffset, size_t dstOffset);

    void map(void **output, size_t offset, vk::MemoryMapFlags mapFlags);

    vk::DeviceAddress getAddress(vk::DispatchLoaderDynamic &dispatchLoaderDynamic);

    void unMap();

    vk::Buffer &getBuffer();

public:
    void destroy() override;
};

#endif


