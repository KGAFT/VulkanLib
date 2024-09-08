//
// Created by kgaft on 3/11/24.
//
#include "Buffer.hpp"

Buffer::Buffer(std::shared_ptr<LogicalDevice> device, vk::BufferCreateInfo *createInfo,
               vk::MemoryPropertyFlags memoryFlags)
        : device(device) {
    initialize(createInfo, memoryFlags);
}

Buffer::Buffer(std::shared_ptr<LogicalDevice> device, size_t size, vk::BufferUsageFlags usageFlags,
               vk::MemoryPropertyFlags memoryFlags) : device(device) {
    initialize(size, usageFlags, memoryFlags);
}

Buffer::Buffer(const std::shared_ptr<LogicalDevice> &device) : device(device) {
}

void Buffer::initialize(size_t size, vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryFlags) {
    if (buffer) {
        destroy();
        destroyed = false;
    }
    vk::BufferCreateInfo *createInfo = createInfos.getObjectInstance();
    createInfo->sType = vk::StructureType::eBufferCreateInfo;
    createInfo->size = size;
    createInfo->usage = usageFlags;
    createInfo->sharingMode = vk::SharingMode::eExclusive;

    initialize(createInfo, memoryFlags);

    createInfos.releaseObjectInstance(createInfo);
}

void Buffer::initialize(vk::BufferCreateInfo *createInfo, vk::MemoryPropertyFlags memoryFlags) {
    if (buffer) {
        destroy();
        destroyed = false;

    }
    vk::Result res = device->getDevice().createBuffer(createInfo, VkLibAlloc::acquireAllocCb().get(), &buffer);
    if (res != vk::Result::eSuccess) {
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

    res = device->getDevice().allocateMemory(info, VkLibAlloc::acquireAllocCb().get(), &bufferMemory);
    if (res != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to allocate buffer memory");
    }
    device->getDevice().bindBufferMemory(buffer, bufferMemory, 0);
    bufferSize = createInfo->size;
    requirements.releaseObjectInstance(memReqs);
    allocInfos.releaseObjectInstance(info);
    flagsInfo.releaseObjectInstance(allocFlags);
}

void Buffer::copyFromBuffer(vk::CommandBuffer cmd, Buffer &source, size_t size, size_t srcOffset, size_t dstOffset) {
    vk::BufferCopy *copy = copyRegions.getObjectInstance();
    copy->size = size;
    copy->srcOffset = srcOffset;
    copy->dstOffset = dstOffset;
    cmd.copyBuffer(source.buffer, buffer, 1, copy);
    copyRegions.releaseObjectInstance(copy);
}

void Buffer::copyToBuffer(vk::CommandBuffer cmd, Buffer &destination, size_t size, size_t srcOffset, size_t dstOffset) {
    vk::BufferCopy *copy = copyRegions.getObjectInstance();
    copy->size = size;
    copy->srcOffset = srcOffset;
    copy->dstOffset = dstOffset;
    cmd.copyBuffer(destination.buffer, buffer, 1, copy);
    copyRegions.releaseObjectInstance(copy);
}

void Buffer::map(void **output, size_t offset, vk::MemoryMapFlags mapFlags) {
    vk::Result res = device->getDevice().mapMemory(bufferMemory, offset, bufferSize, mapFlags, output);
    if (res != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to map buffer");
    }
}

vk::DeviceAddress Buffer::getAddress(vk::DispatchLoaderDynamic &dispatchLoaderDynamic) {
    addressInfo.buffer = buffer;
    return device->getDevice().getBufferAddressKHR(addressInfo, dispatchLoaderDynamic);
}

void Buffer::unMap() {
    device->getDevice().unmapMemory(bufferMemory);
}

vk::Buffer &Buffer::getBuffer() {
    return buffer;
}

void Buffer::destroy() {
    destroyed = true;
    device->getDevice().destroyBuffer(buffer);
    device->getDevice().freeMemory(bufferMemory);
    addressInfo = vk::BufferDeviceAddressInfo{};
}
