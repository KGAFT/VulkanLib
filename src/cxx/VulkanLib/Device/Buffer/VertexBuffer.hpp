//
// Created by kgaft on 11/27/23.
//
#pragma once

#include "Buffer.hpp"

class VertexBuffer {
private:
    static inline SeriesObject<vk::BufferCreateInfo> createInfos = SeriesObject<vk::BufferCreateInfo>();
public:
    VertexBuffer(std::shared_ptr<LogicalDevice> device, void *vertices, uint32_t verticesAmount, size_t stepSize)
            : device(device), vertexCount(verticesAmount) {
        uint32_t queueIndices[] = {device->getQueueByType(vk::QueueFlagBits::eGraphics)->getIndex()};


        vk::BufferCreateInfo *createInfo = createInfos.getObjectInstance();
        createInfo->sType = vk::StructureType::eBufferCreateInfo;
        createInfo->size = verticesAmount * stepSize;
        createInfo->usage = vk::BufferUsageFlagBits::eTransferSrc;
        createInfo->sharingMode = vk::SharingMode::eExclusive;
        createInfo->pQueueFamilyIndices = queueIndices;
        createInfo->queueFamilyIndexCount = 1;

        void *mapPoint = nullptr;

        Buffer stagingBuffer(device, createInfo,
                             vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        stagingBuffer.map(&mapPoint, 0, vk::MemoryMapFlags());
        memcpy(mapPoint, vertices, verticesAmount * stepSize);
        stagingBuffer.unMap();

        createInfo->usage = vk::BufferUsageFlagBits::eVertexBuffer |
                            vk::BufferUsageFlagBits::eTransferDst;

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
    uint32_t vertexCount;
public:
    void bind(vk::CommandBuffer cmd) {
        vk::DeviceSize offset = 0;
        cmd.bindVertexBuffers(0, buffer->getBuffer(), offset);
    }

    void drawAll(vk::CommandBuffer cmd) {
        cmd.draw(vertexCount, 1, 0, 0);
    }
};


