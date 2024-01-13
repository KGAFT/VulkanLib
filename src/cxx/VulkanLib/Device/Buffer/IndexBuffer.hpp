//
// Created by kgaft on 11/27/23.
//

#pragma once

#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "Buffer.hpp"

class IndexBuffer {
private:
    static inline SeriesObject<vk::BufferCreateInfo> createInfos = SeriesObject<vk::BufferCreateInfo>();
public:
    IndexBuffer(std::shared_ptr<LogicalDevice> device, void *indices, uint32_t indexCount, vk::IndexType indexType,
                bool forRayTracing)
            : device(device), indexCount(indexCount), indexType(indexType) {
        uint32_t queueIndices[] = {device->getQueueByType(vk::QueueFlagBits::eGraphics)->getIndex()};


        vk::BufferCreateInfo *createInfo = createInfos.getObjectInstance();
        createInfo->sType = vk::StructureType::eBufferCreateInfo;
        createInfo->size = indexCount * sizeof(uint32_t);
        createInfo->usage = vk::BufferUsageFlagBits::eTransferSrc;
        createInfo->sharingMode = vk::SharingMode::eExclusive;
        createInfo->pQueueFamilyIndices = queueIndices;
        createInfo->queueFamilyIndexCount = 1;

        void *mapPoint = nullptr;

        Buffer stagingBuffer(device, createInfo,
                             vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        stagingBuffer.map(&mapPoint, 0, vk::MemoryMapFlags());
        memcpy(mapPoint, indices, indexCount * sizeof(uint32_t));
        stagingBuffer.unMap();

        createInfo->usage = vk::BufferUsageFlagBits::eIndexBuffer |
                            vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress | (
                forRayTracing ? vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR
                              : vk::BufferUsageFlags());;

        buffer = std::make_shared<Buffer>(device, createInfo, vk::MemoryPropertyFlagBits::eDeviceLocal);
        vk::CommandBuffer cmd = device->getQueueByType(vk::QueueFlagBits::eGraphics)->beginSingleTimeCommands();
        buffer->copyFromBuffer(cmd, stagingBuffer, createInfo->size, 0, 0);
        device->getQueueByType(vk::QueueFlagBits::eGraphics)->endSingleTimeCommands(cmd);
        createInfos.releaseObjectInstance(createInfo);
        stagingBuffer.destroy();
    }

private:
    std::shared_ptr<LogicalDevice> device;
    std::shared_ptr<Buffer> buffer;
    uint32_t indexCount;
    vk::IndexType indexType;
public:
    void bind(vk::CommandBuffer cmd) {
        vk::DeviceSize offset = 0;
        cmd.bindIndexBuffer(buffer->getBuffer(), 0, vk::IndexType::eUint32);
    }

    void drawAll(vk::CommandBuffer cmd) {
        cmd.drawIndexed(indexCount, 1, 0, 0, 0);
    }

    vk::IndexType getIndexType() const {
        return indexType;
    }

    vk::DeviceAddress getBufferAddress(vk::DispatchLoaderDynamic &loaderDynamic) {
        return buffer->getAddress(loaderDynamic);
    }
};


