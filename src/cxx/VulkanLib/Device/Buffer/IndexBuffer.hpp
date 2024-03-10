//
// Created by kgaft on 11/27/23.
//

#ifndef VULKANLIB_INDEXBUFFER_HPP
#define VULKANLIB_INDEXBUFFER_HPP

#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "Buffer.hpp"

class IndexBuffer : public IDestroyableObject {
private:
    static inline auto createInfos = SeriesObject<vk::BufferCreateInfo>();
public:
    IndexBuffer(std::shared_ptr<LogicalDevice> device, void *indices, uint32_t indexCount, vk::IndexType indexType,
                bool forRayTracing);

private:
    std::shared_ptr<LogicalDevice> device;
    std::shared_ptr<Buffer> buffer;
    uint32_t indexCount;
    vk::IndexType indexType;
public:
    void bind(vk::CommandBuffer cmd);

    void drawAll(vk::CommandBuffer cmd);

    vk::IndexType getIndexType() const;

    vk::DeviceAddress getBufferAddress(vk::DispatchLoaderDynamic &loaderDynamic);

    uint32_t getIndexCount() const;

    void destroy() override;
};

#endif

