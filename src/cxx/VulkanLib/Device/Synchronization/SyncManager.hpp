//
// Created by kgaft on 11/18/23.
//
#pragma once

#include "ThreeFrameSynchronization.hpp"

class SyncManager : IDestroyableObject{
public:
    SyncManager(std::shared_ptr<LogicalDevice> device, std::shared_ptr<SwapChain> swapChain, std::shared_ptr<LogicalQueue> queue, uint32_t maxFramesInFlight) : sync(device, queue, maxFramesInFlight), device(device),
                                                                                    swapChain(swapChain), queue(queue) {
        createCommandBuffers(maxFramesInFlight);
    }

private:
    std::vector<vk::CommandBuffer> commandBuffers;
    ThreeFrameSynchronization sync;
    std::shared_ptr<LogicalDevice> device;
    std::shared_ptr<SwapChain> swapChain;
    std::shared_ptr<LogicalQueue> queue;
    uint32_t currentCmd;
    vk::CommandBufferBeginInfo beginInfo{};
    bool stop = false;
public:
    vk::CommandBuffer beginRender(uint32_t& outCurrentCmd) {
        if(!stop){
            currentCmd = sync.prepareForNextImage(swapChain);
            commandBuffers[currentCmd].begin(beginInfo);
            outCurrentCmd = currentCmd;
            return commandBuffers[currentCmd];
        }
        return nullptr;
    }

    void endRender() {
        if(!stop){
            commandBuffers[currentCmd].end();
            sync.submitCommandBuffers(&commandBuffers[currentCmd], swapChain, &currentCmd);
        }

    }

    bool isStop() const {
        return stop;
    }

    void setStop(bool stop) {
        if(stop){
            sync.waitStop();
        }
        SyncManager::stop = stop;
    }

private:
    void createCommandBuffers(uint32_t maxFramesInFlight) {
        commandBuffers.resize(maxFramesInFlight);

        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandPool = queue->getCommandPool();
        allocInfo.commandBufferCount = maxFramesInFlight;
        uint32_t c = 0;
        for (auto &item: device->getDevice().allocateCommandBuffers(allocInfo)) {
            commandBuffers[c] = item;
            c++;
        }
    }
public:
    void destroy() override {
        destroyed = true;
    }
};


