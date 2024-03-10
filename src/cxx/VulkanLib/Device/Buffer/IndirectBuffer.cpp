//
// Created by kgaft on 3/11/24.
//
#include "IndirectBuffer.hpp"

IndirectBuffer::IndirectBuffer(std::shared_ptr<LogicalDevice> device, uint32_t structuresAmount, size_t sizeOfStructure)
        : structuresAmount(structuresAmount), sizeOfStructure(sizeOfStructure) {


    auto createInfo = createInfos.getObjectInstance();
    createInfo->sType = vk::StructureType::eBufferCreateInfo;
    createInfo->size = structuresAmount * sizeOfStructure;
    createInfo->usage = vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eTransferDst |
                        vk::BufferUsageFlagBits::eTransferSrc;
    createInfo->sharingMode = vk::SharingMode::eExclusive;
    buffer = std::make_shared<Buffer>(device, createInfo, vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);
    createInfos.releaseObjectInstance(createInfo);
    buffer->map(&mapPoint, 0, vk::MemoryMapFlags());
}

void *IndirectBuffer::getMapPoint() {
    return mapPoint;
}

vk::Buffer &IndirectBuffer::getBuffer() {
    return buffer->getBuffer();
}

uint32_t IndirectBuffer::getStructuresAmount() const {
    return structuresAmount;
}

size_t IndirectBuffer::getSizeOfStructure() const {
    return sizeOfStructure;
}

void IndirectBuffer::destroy() {
    destroyed = true;
    buffer->unMap();
    buffer->destroy();
}
