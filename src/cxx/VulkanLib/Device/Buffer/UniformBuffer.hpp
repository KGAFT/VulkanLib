//
// Created by kgaft on 12/1/23.
//
#pragma once

#include "Buffer.hpp"

class UniformBuffer {
private:
    static inline SeriesObject<vk::BufferCreateInfo> createInfos = SeriesObject<vk::BufferCreateInfo>();

public:
    UniformBuffer(std::shared_ptr<LogicalDevice> device, size_t bufferSize){
        uint32_t queueIndices[] = {device->getQueueByType(vk::QueueFlagBits::eGraphics)->getIndex()};


        vk::BufferCreateInfo *createInfo = createInfos.getObjectInstance();
        createInfo->sType = vk::StructureType::eBufferCreateInfo;
        createInfo->usage = vk::BufferUsageFlagBits::eUniformBuffer;
        createInfo->size = bufferSize;
        createInfo->pQueueFamilyIndices = queueIndices;
        createInfo->queueFamilyIndexCount = 1;
        createInfo->sharingMode = vk::SharingMode::eExclusive;
        base = new Buffer(device, createInfo, vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);

        base->map(&mapPoint, 0, vk::MemoryMapFlags());
        createInfos.releaseObjectInstance(createInfo);
    }
private:
    Buffer* base;
    void* mapPoint;
public:
    void *getMapPoint()  {
        return mapPoint;
    }
    vk::Buffer getBuffer(){
        return base->getBuffer();
    }
};


