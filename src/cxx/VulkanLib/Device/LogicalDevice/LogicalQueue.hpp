//
// Created by kgaft on 11/5/23.
//

#pragma once

#include <vulkan/vulkan.hpp>

class LogicalQueue : IDestroyableObject{
    friend class LogicalDevice;

public:
    LogicalQueue(vk::Queue queue, vk::Device device, bool supportPresentation, vk::QueueFlagBits queueType, unsigned int index) : queue(
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
        commandPool = device.createCommandPool(poolInfo);
    }

private:
    vk::Queue queue;
    bool supportPresentation;
    vk::QueueFlagBits queueType;
    vk::CommandPool commandPool;
    vk::Device device;
    unsigned int index;
    vk::CommandBufferAllocateInfo allocInfo{};
    vk::CommandBufferBeginInfo beginInfo{};
    vk::SubmitInfo submitInfo{};


public:
    vk::CommandBuffer beginSingleTimeCommands()
    {
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(allocInfo)[0];



        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        commandBuffer.begin(beginInfo);
        return commandBuffer;
    }
    void endSingleTimeCommands(vk::CommandBuffer commandBuffer){
        commandBuffer.end();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        queue.submit(submitInfo);
        queue.waitIdle();
        device.freeCommandBuffers(commandPool, commandBuffer);
    }


    const vk::Queue &getQueue() const {
        return queue;
    }

    bool isSupportPresentation() const {
        return supportPresentation;
    }

    vk::QueueFlagBits getQueueType() const {
        return queueType;
    }

    const vk::CommandPool &getCommandPool() const {
        return commandPool;
    }

    unsigned int getIndex() const {
        return index;
    }

private:
    void destroy() override {
        destroyed = true;
        device.destroyCommandPool(commandPool);
    }
};


