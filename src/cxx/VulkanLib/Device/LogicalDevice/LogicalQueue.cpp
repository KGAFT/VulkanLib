//
// Created by kgaft on 3/11/24.
//

#include "LogicalQueue.hpp"
#include "VulkanLib/VulkanLibAllocationCallback.h"

LogicalQueue::LogicalQueue(vk::Queue queue, vk::Device device, bool supportPresentation, vk::QueueFlags queueType,
                           unsigned int index) : queue(
        queue),
                                                 supportPresentation(
                                                         supportPresentation),
                                                 queueType(
                                                         queueType),
                                                 device(device), index(index) {
    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.queueFamilyIndex = index;
    poolInfo.flags =
            vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    commandPool = device.createCommandPool(poolInfo, VkLibAlloc::acquireAllocCb().get());
}

vk::CommandBuffer LogicalQueue::beginSingleTimeCommands() {
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(allocInfo)[0];



    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    commandBuffer.begin(beginInfo);
    return commandBuffer;
}

void LogicalQueue::endSingleTimeCommands(vk::CommandBuffer commandBuffer) {
    commandBuffer.end();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    queue.submit(submitInfo);
    queue.waitIdle();
    device.freeCommandBuffers(commandPool, commandBuffer);
}

const vk::Queue &LogicalQueue::getQueue() const {
    return queue;
}

bool LogicalQueue::isSupportPresentation() const {
    return supportPresentation;
}

vk::QueueFlags LogicalQueue::getQueueType() const {
    return queueType;
}

const vk::CommandPool &LogicalQueue::getCommandPool() const {
    return commandPool;
}

unsigned int LogicalQueue::getIndex() const {
    return index;
}

void LogicalQueue::destroy() {
    destroyed = true;
    device.destroyCommandPool(commandPool);
}
