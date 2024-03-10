//
// Created by kgaft on 12/1/23.
//
#ifndef VULKANLIB_UNIFORMBUFFER_HPP
#define VULKANLIB_UNIFORMBUFFER_HPP

#include "Buffer.hpp"

class UniformBuffer : public IDestroyableObject {
private:
    static inline auto createInfos = SeriesObject<vk::BufferCreateInfo>();

public:
    UniformBuffer(std::shared_ptr<LogicalDevice> device, size_t bufferSize);

private:
    Buffer *base;
    void *mapPoint;
public:
    void *getMapPoint();

    vk::Buffer getBuffer();

public:
    void destroy() override;
};

#endif
