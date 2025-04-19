//
// Created by kgaft on 11/27/23.
//
#ifndef VULKANLIB_VERTEXBUFFER_HPP
#define VULKANLIB_VERTEXBUFFER_HPP

#include "Buffer.hpp"

class VertexBuffer : public IDestroyableObject {
private:
    static inline auto createInfos = SeriesObject<vk::BufferCreateInfo>();
public:
    VertexBuffer(std::shared_ptr<LogicalDevice> device, void *vertices, uint32_t verticesAmount, size_t stepSize,
                 vk::Format format, bool forRayTracing);

private:
    std::shared_ptr<LogicalDevice> device;
    std::shared_ptr<Buffer> buffer;
    uint32_t vertexCount;
    vk::Format format;
    size_t stepSize;
    size_t verticesAmount;
public:
    void bind(vk::CommandBuffer cmd);

    size_t getStepSize() const;

    void drawAll(vk::CommandBuffer cmd);

    vk::Format getFormat() const;

    size_t getVerticesAmount() const;

    vk::DeviceAddress getBufferAddress(vk::detail::DispatchLoaderDynamic &loaderDynamic);

    void destroy() override;
};

#endif

