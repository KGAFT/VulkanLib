//
// Created by kgaft on 11/19/23.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include "VulkanLib/MemoryUtils/IDestroyableObject.hpp"
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "VulkanLib/MemoryUtils/SeriesObject.hpp"
#include <memory>

class Buffer : public IDestroyableObject {
private:
    static inline SeriesObject<vk::MemoryRequirements> requirements = SeriesObject<vk::MemoryRequirements>();
    static inline SeriesObject<vk::MemoryAllocateInfo> allocInfos = SeriesObject<vk::MemoryAllocateInfo>();
    static inline SeriesObject<vk::MemoryAllocateFlagsInfo> flagsInfo = SeriesObject<vk::MemoryAllocateFlagsInfo>();

public:
    Buffer(std::shared_ptr<LogicalDevice> device, vk::BufferCreateInfo *createInfo, vk::MemoryPropertyFlags memoryFlags)
            : device(device) {
        vk::Result res = device->getDevice().createBuffer(createInfo, nullptr, &buffer);
        if(res!=vk::Result::eSuccess){
            throw std::runtime_error("Failed to create buffer");
        }
        vk::MemoryRequirements *memReqs = requirements.getObjectInstance();
        device->getDevice().getBufferMemoryRequirements(buffer, memReqs);
        vk::MemoryAllocateInfo *info = allocInfos.getObjectInstance();
        info->sType = vk::StructureType::eMemoryAllocateInfo;
        info->allocationSize = memReqs->size;
        info->memoryTypeIndex = device->findMemoryType(memReqs->memoryTypeBits, memoryFlags);

        auto allocFlags = flagsInfo.getObjectInstance();
        allocFlags->sType = vk::StructureType::eMemoryAllocateFlagsInfo;
        allocFlags->flags = vk::MemoryAllocateFlagBits::eDeviceAddress;

        info->pNext = allocFlags;

        res = device->getDevice().allocateMemory(info, nullptr, &bufferMemory);
        if(res!=vk::Result::eSuccess){
            throw std::runtime_error("Failed to allocate buffer memory");
        }
        device->getDevice().bindBufferMemory(buffer, bufferMemory, 0);
        bufferSize = createInfo->size;
        requirements.releaseObjectInstance(memReqs);
        allocInfos.releaseObjectInstance(info);
        flagsInfo.releaseObjectInstance(allocFlags);
    }

private:
    vk::Buffer buffer;
    vk::DeviceMemory bufferMemory;
    std::shared_ptr<LogicalDevice> device;
    SeriesObject<vk::BufferCopy> copyRegions;
    size_t bufferSize;
    vk::BufferDeviceAddressInfo addressInfo{};
public:
    void copyFromBuffer(vk::CommandBuffer cmd, Buffer &source, size_t size, size_t srcOffset, size_t dstOffset) {
        vk::BufferCopy *copy = copyRegions.getObjectInstance();
        copy->size = size;
        copy->srcOffset = srcOffset;
        copy->dstOffset = dstOffset;
        cmd.copyBuffer(source.buffer, buffer, 1, copy);
        copyRegions.releaseObjectInstance(copy);
    }

    void copyToBuffer(vk::CommandBuffer cmd, Buffer &destination, size_t size, size_t srcOffset, size_t dstOffset) {
        vk::BufferCopy *copy = copyRegions.getObjectInstance();
        copy->size = size;
        copy->srcOffset = srcOffset;
        copy->dstOffset = dstOffset;
        cmd.copyBuffer(destination.buffer, buffer, 1, copy);
        copyRegions.releaseObjectInstance(copy);
    }

    void map(void **output, size_t offset, vk::MemoryMapFlags mapFlags) {
        vk::Result res = device->getDevice().mapMemory(bufferMemory, offset, bufferSize, mapFlags, output);
        if(res!=vk::Result::eSuccess){
            throw std::runtime_error("Failed to map buffer");
        }
    }
    vk::DeviceAddress getAddress(vk::DispatchLoaderDynamic& dispatchLoaderDynamic){
         addressInfo.buffer = buffer;
         return device->getDevice().getBufferAddressKHR(addressInfo, dispatchLoaderDynamic);
    }
    void unMap() {
        device->getDevice().unmapMemory(bufferMemory);
    }

    vk::Buffer &getBuffer() {
        return buffer;
    }

public:
    void destroy() override {
        destroyed = true;
        device->getDevice().destroyBuffer(buffer);
        device->getDevice().freeMemory(bufferMemory);
    }
};


