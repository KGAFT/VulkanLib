//
// Created by kgaft on 6/9/24.
//

#ifndef ONEFRAMESYNCCOMPUTE_HPP
#define ONEFRAMESYNCCOMPUTE_HPP
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "VulkanLib/MemoryUtils/SeriesObject.hpp"


class OneFrameSyncCompute : public IDestroyableObject {
private:
    static inline SeriesObject<vk::SubmitInfo> submitInfos = SeriesObject<vk::SubmitInfo>();
    static inline SeriesObject<vk::CommandBufferBeginInfo> beginInfos = SeriesObject<vk::CommandBufferBeginInfo>();
public:
    OneFrameSyncCompute(std::shared_ptr<LogicalDevice> device) : device(device), computeQueue(device->getQueueByType(vk::QueueFlagBits::eCompute))
    {
        createSyncObjects();
    }
private:
    std::shared_ptr<LogicalQueue> computeQueue;
    std::shared_ptr<LogicalDevice> device;
    vk::Semaphore availableSemaphore;
    vk::Semaphore waitSemaphore;
    vk::CommandBuffer cmd;
    vk::Fence fence;
    bool firstFrame = true;
public:

    void destroy() override
    {

        device->getDevice().waitIdle();
        device->getDevice().destroySemaphore(availableSemaphore);
        device->getDevice().destroySemaphore(waitSemaphore);
        device->getDevice().destroyFence(fence);
        destroyed = true;
    }
    vk::CommandBuffer beginCommandBuffer()
    {
        auto beginInfo = beginInfos.getObjectInstance();
        beginInfo->sType = vk::StructureType::eCommandBufferBeginInfo;
        if (cmd.begin(beginInfo)!=vk::Result::eSuccess)
        {
            beginInfos.releaseObjectInstance(beginInfo);
            throw std::runtime_error("failed to begin recording command buffer!");
        }
        beginInfos.releaseObjectInstance(beginInfo);
        return cmd;
    }

    void submitCommandBuffer(vk::CommandBuffer commandBuffer)
    {
        if(device->getDevice().resetFences(1, &fence)!=vk::Result::eSuccess) {
            throw std::runtime_error("Failed to reset fences");
        }
        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eComputeShader};
        auto submitInfo = submitInfos.getObjectInstance();
        submitInfo->sType = vk::StructureType::eSubmitInfo;
        submitInfo->signalSemaphoreCount = 1;
        submitInfo->waitSemaphoreCount = !firstFrame;
        submitInfo->pWaitSemaphores = firstFrame ? VK_NULL_HANDLE : &waitSemaphore;
        submitInfo->pSignalSemaphores = &availableSemaphore;
        submitInfo->commandBufferCount = 1;
        submitInfo->pCommandBuffers = &commandBuffer;
        submitInfo->pWaitDstStageMask = waitStages;
        computeQueue->getQueue().submit(*submitInfo, fence);
        if(device->getDevice().waitForFences(1, &fence, VK_TRUE, UINT64_MAX)!=vk::Result::eSuccess) {
            throw std::runtime_error("Failed to wait for fences");
        }
        firstFrame = false;
        submitInfos.releaseObjectInstance(submitInfo);
        std::swap(availableSemaphore, waitSemaphore);
    }

private:
    void createSyncObjects()
    {
        vk::SemaphoreCreateInfo semaphoreInfo = {};
        if(device->getDevice().createSemaphore(&semaphoreInfo, nullptr, &availableSemaphore)!=vk::Result::eSuccess) {
            throw std::runtime_error("Failed to create semaphore");
        }
        if(device->getDevice().createSemaphore(&semaphoreInfo, nullptr,  &waitSemaphore)!=vk::Result::eSuccess) {
            throw std::runtime_error("Failed to create semaphore");
        }
        vk::FenceCreateInfo fenceInfo = {};
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
        if(device->getDevice().createFence( &fenceInfo, nullptr, &fence)!=vk::Result::eSuccess) {
            throw std::runtime_error("Failed to create fence");
        }

        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandPool = computeQueue->getCommandPool();
        allocInfo.commandBufferCount = 1;
        cmd = device->getDevice().allocateCommandBuffers(allocInfo)[0];
    }
};



#endif //ONEFRAMESYNCCOMPUTE_HPP
