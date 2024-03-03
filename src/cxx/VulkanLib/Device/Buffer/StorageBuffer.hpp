//
// Created by kgaft on 1/4/24.
//

#ifndef VULKANRENDERENGINE_STORAGEBUFFER_HPP
#define VULKANRENDERENGINE_STORAGEBUFFER_HPP


#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "Buffer.hpp"

class StorageBuffer : public IDestroyableObject {

public:
    StorageBuffer(std::shared_ptr<LogicalDevice> device, size_t bufferSize, vk::BufferUsageFlags additionalFlags) {


        buffer = std::make_shared<Buffer>(device, bufferSize, vk::BufferUsageFlagBits::eStorageBuffer |
                                                              vk::BufferUsageFlagBits::eTransferDst |
                                                              vk::BufferUsageFlagBits::eTransferSrc | additionalFlags,
                                          vk::MemoryPropertyFlagBits::eHostVisible |
                                          vk::MemoryPropertyFlagBits::eHostCoherent);
        buffer->map(&mapPoint, 0, vk::MemoryMapFlags());
    }

private:
    std::shared_ptr<Buffer> buffer;
    void *mapPoint;
public:
    void *getMapPoint() {
        return mapPoint;
    }

    vk::Buffer &getBuffer() {
        return buffer->getBuffer();
    }

    void destroy() override {
        destroyed = true;
        buffer->unMap();
        buffer->destroy();
    }
};


#endif //VULKANRENDERENGINE_STORAGEBUFFER_HPP
