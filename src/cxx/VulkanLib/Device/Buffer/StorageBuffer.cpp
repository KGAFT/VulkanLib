#include "StorageBuffer.hpp"

StorageBuffer::StorageBuffer(std::shared_ptr<LogicalDevice> device, size_t bufferSize,
                             vk::BufferUsageFlags additionalFlags) : bufferSize(bufferSize) {


    buffer = std::make_shared<Buffer>(device, bufferSize, vk::BufferUsageFlagBits::eStorageBuffer |
                                                          vk::BufferUsageFlagBits::eTransferDst |
                                                          vk::BufferUsageFlagBits::eTransferSrc | additionalFlags,
                                      vk::MemoryPropertyFlagBits::eHostVisible |
                                      vk::MemoryPropertyFlagBits::eHostCoherent);
    buffer->map(&mapPoint, 0, vk::MemoryMapFlags());
}

void *StorageBuffer::getMapPoint() {
    return mapPoint;
}

size_t StorageBuffer::getBufferSize() const {
    return bufferSize;
}

vk::Buffer &StorageBuffer::getBuffer() {
    return buffer->getBuffer();
}

std::shared_ptr<Buffer> StorageBuffer::getBase(){
    return buffer;
}


void StorageBuffer::destroy() {
    destroyed = true;
    buffer->unMap();
    buffer->destroy();
}
