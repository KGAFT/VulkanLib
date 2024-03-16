//
// Created by kgaft on 1/4/24.
//

#ifndef VULKANLIB_STORAGEBUFFER_HPP
#define VULKANLIB_STORAGEBUFFER_HPP


#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "Buffer.hpp"

class StorageBuffer : public IDestroyableObject {

public:
    StorageBuffer(std::shared_ptr<LogicalDevice> device, size_t bufferSize, vk::BufferUsageFlags additionalFlags);

private:
    std::shared_ptr<Buffer> buffer;
    void *mapPoint;
    size_t bufferSize;
public:
    void *getMapPoint();

    size_t getBufferSize() const;

    vk::Buffer &getBuffer();

    std::shared_ptr<Buffer> getBase();

    void destroy() override;
};


#endif //VULKANLIB_STORAGEBUFFER_HPP
