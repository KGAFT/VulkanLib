//
// Created by kgaft on 3/11/24.
//
#include "UniformBuffer.hpp"

UniformBuffer::UniformBuffer(std::shared_ptr<LogicalDevice> device, size_t bufferSize) {


    vk::BufferCreateInfo *createInfo = createInfos.getObjectInstance();
    createInfo->sType = vk::StructureType::eBufferCreateInfo;
    createInfo->usage = vk::BufferUsageFlagBits::eUniformBuffer;
    createInfo->size = bufferSize;

    createInfo->sharingMode = vk::SharingMode::eExclusive;
    base = new Buffer(device, createInfo,
                      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    base->map(&mapPoint, 0, vk::MemoryMapFlags());
    createInfos.releaseObjectInstance(createInfo);
}

void *UniformBuffer::getMapPoint() {
    return mapPoint;
}

vk::Buffer UniformBuffer::getBuffer() {
    return base->getBuffer();
}

void UniformBuffer::destroy() {
    base->unMap();
    base->destroy();
    destroyed = true;
}
