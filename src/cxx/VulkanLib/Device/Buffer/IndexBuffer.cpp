//
// Created by kgaft on 3/11/24.
//
#include "IndexBuffer.hpp"

IndexBuffer::IndexBuffer(std::shared_ptr<LogicalDevice> device, void *indices, uint32_t indexCount,
                         vk::IndexType indexType, bool forRayTracing)
        : device(device), indexCount(indexCount), indexType(indexType) {


    vk::BufferCreateInfo *createInfo = createInfos.getObjectInstance();
    createInfo->sType = vk::StructureType::eBufferCreateInfo;
    createInfo->size = indexCount * sizeof(uint32_t);
    createInfo->usage = vk::BufferUsageFlagBits::eTransferSrc;
    createInfo->sharingMode = vk::SharingMode::eExclusive;


    void *mapPoint = nullptr;

    Buffer stagingBuffer(device, createInfo,
                         vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    stagingBuffer.map(&mapPoint, 0, vk::MemoryMapFlags());
    memcpy(mapPoint, indices, indexCount * sizeof(uint32_t));
    stagingBuffer.unMap();

    createInfo->usage = vk::BufferUsageFlagBits::eIndexBuffer |
                        vk::BufferUsageFlagBits::eTransferDst | (
                                forRayTracing ? vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
                                                vk::BufferUsageFlagBits::eShaderDeviceAddress
                                              : vk::BufferUsageFlags());;

    buffer = std::make_shared<Buffer>(device, createInfo, vk::MemoryPropertyFlagBits::eDeviceLocal);
    vk::CommandBuffer cmd = device->getQueueByType(vk::QueueFlagBits::eGraphics)->beginSingleTimeCommands();
    buffer->copyFromBuffer(cmd, stagingBuffer, createInfo->size, 0, 0);
    device->getQueueByType(vk::QueueFlagBits::eGraphics)->endSingleTimeCommands(cmd);
    createInfos.releaseObjectInstance(createInfo);
    stagingBuffer.destroy();
}

void IndexBuffer::bind(vk::CommandBuffer cmd) {
    cmd.bindIndexBuffer(buffer->getBuffer(), 0, vk::IndexType::eUint32);
}

void IndexBuffer::drawAll(vk::CommandBuffer cmd) {
    cmd.drawIndexed(indexCount, 1, 0, 0, 0);
}

vk::IndexType IndexBuffer::getIndexType() const {
    return indexType;
}

vk::DeviceAddress IndexBuffer::getBufferAddress(vk::detail::DispatchLoaderDynamic &loaderDynamic) {
    return buffer->getAddress(loaderDynamic);
}

uint32_t IndexBuffer::getIndexCount() const {
    return indexCount;
}

void IndexBuffer::destroy() {
    destroyed = true;
    buffer->destroy();
}
