//
// Created by kgaft on 11/18/23.
//
#pragma once

#include "ThreeFrameSynchronization.hpp"

class SyncManager {
public:
    SyncManager(LogicalDevice &device, SwapChain &swapChain, LogicalQueue &queue) : sync(device, queue), device(device),
                                                                                    swapChain(swapChain), queue(queue) {
        createCommandBuffers();
    }

private:
    std::vector<vk::CommandBuffer> commandBuffers;
    ThreeFrameSynchronization sync;
    LogicalDevice &device;
    SwapChain &swapChain;
    LogicalQueue &queue;
    uint32_t currentCmd;
    vk::CommandBufferBeginInfo beginInfo{};
public:
    vk::CommandBuffer beginRender(uint32_t& outCurrentCmd) {
        currentCmd = sync.prepareForNextImage(swapChain);
        commandBuffers[currentCmd].begin(beginInfo);
        outCurrentCmd = currentCmd;
        return commandBuffers[currentCmd];
    }

    void endRender() {
        commandBuffers[currentCmd].end();
        sync.submitCommandBuffers(&commandBuffers[currentCmd], swapChain, &currentCmd);
    }

private:
    void createCommandBuffers() {
        commandBuffers.resize(3);

        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandPool = queue.getCommandPool();
        allocInfo.commandBufferCount = 3;
        uint32_t c = 0;
        for (auto &item: device.getDevice().allocateCommandBuffers(allocInfo)) {
            commandBuffers[c] = item;
            c++;
        }
    }
};


