//
// Created by kgaft on 11/5/23.
//

#ifndef VULKANLIB_LOGICALQUEUE_HPP
#define VULKANLIB_LOGICALQUEUE_HPP

#include <vulkan/vulkan.hpp>
#include "VulkanLib/MemoryUtils/IDestroyableObject.hpp"

class LogicalQueue : IDestroyableObject{
    friend class LogicalDevice;

public:
    LogicalQueue(vk::Queue queue, vk::Device device, bool supportPresentation, vk::QueueFlags queueType, unsigned int index);

private:
    vk::Queue queue;
    bool supportPresentation;
    vk::QueueFlags queueType;
    vk::CommandPool commandPool;
    vk::Device device;
    unsigned int index;
    vk::CommandBufferAllocateInfo allocInfo{};
    vk::CommandBufferBeginInfo beginInfo{};
    vk::SubmitInfo submitInfo{};


public:
    vk::CommandBuffer beginSingleTimeCommands();

    void endSingleTimeCommands(vk::CommandBuffer commandBuffer);


    const vk::Queue &getQueue() const;

    bool isSupportPresentation() const;

    vk::QueueFlags getQueueType() const;

    const vk::CommandPool &getCommandPool() const;

    unsigned int getIndex() const;

private:
    void destroy() override;
public:
    virtual ~LogicalQueue() = default;
};

#endif