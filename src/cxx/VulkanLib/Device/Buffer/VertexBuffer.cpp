//
// Created by kgaft on 3/11/24.
//
#include "VertexBuffer.hpp"

VertexBuffer::VertexBuffer(std::shared_ptr<LogicalDevice> device, void *vertices, uint32_t verticesAmount,
                           size_t stepSize, vk::Format format, bool forRayTracing)
        : device(device), vertexCount(verticesAmount), format(format), stepSize(stepSize),
          verticesAmount(verticesAmount) {


    vk::BufferCreateInfo *createInfo = createInfos.getObjectInstance();
    createInfo->sType = vk::StructureType::eBufferCreateInfo;
    createInfo->size = verticesAmount * stepSize;
    createInfo->usage = vk::BufferUsageFlagBits::eTransferSrc;
    createInfo->sharingMode = vk::SharingMode::eExclusive;


    void *mapPoint = nullptr;

    Buffer stagingBuffer(device, createInfo,
                         vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    stagingBuffer.map(&mapPoint, 0, vk::MemoryMapFlags());
    memcpy(mapPoint, vertices, verticesAmount * stepSize);
    stagingBuffer.unMap();

    createInfo->usage = vk::BufferUsageFlagBits::eVertexBuffer |
                        vk::BufferUsageFlagBits::eTransferDst |
                        (forRayTracing ? vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
                                         vk::BufferUsageFlagBits::eShaderDeviceAddress
                                       : vk::BufferUsageFlags());

    buffer = std::make_shared<Buffer>(device, createInfo, vk::MemoryPropertyFlagBits::eDeviceLocal);
    vk::CommandBuffer cmd = device->getQueueByType(vk::QueueFlagBits::eGraphics)->beginSingleTimeCommands();
    buffer->copyFromBuffer(cmd, stagingBuffer, createInfo->size, 0, 0);
    device->getQueueByType(vk::QueueFlagBits::eGraphics)->endSingleTimeCommands(cmd);
    createInfos.releaseObjectInstance(createInfo);
    stagingBuffer.destroy();
}

void VertexBuffer::bind(vk::CommandBuffer cmd) {
    vk::DeviceSize offset = 0;
    cmd.bindVertexBuffers(0, buffer->getBuffer(), offset);
}

size_t VertexBuffer::getStepSize() const {
    return stepSize;
}

void VertexBuffer::drawAll(vk::CommandBuffer cmd) {
    cmd.draw(vertexCount, 1, 0, 0);
}

vk::Format VertexBuffer::getFormat() const {
    return format;
}

size_t VertexBuffer::getVerticesAmount() const {
    return verticesAmount;
}

vk::DeviceAddress VertexBuffer::getBufferAddress(vk::DispatchLoaderDynamic &loaderDynamic) {
    return buffer->getAddress(loaderDynamic);
}

void VertexBuffer::destroy() {
    destroyed = true;
    buffer->destroy();
}
