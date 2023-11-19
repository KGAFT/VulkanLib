//
// Created by kgaft on 11/18/23.
//
#pragma once

#include <vector>
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "VulkanLib/Device/SwapChain/SwapChain.hpp"


class ThreeFrameSynchronization : IDestroyableObject {
public:
    ThreeFrameSynchronization(LogicalDevice &device, LogicalQueue &graphicsQueue, uint32_t maxFramesInFlight) : maxFramesInFlight(maxFramesInFlight), device(device),
                                                                                    presentQueue(graphicsQueue) {
        createSyncObjects();
    }

private:
    uint32_t maxFramesInFlight;
    std::vector<vk::Semaphore> imageAvailableSemaphores;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    std::vector<vk::Fence> inFlightFences;
    std::vector<vk::Fence> imagesInFlight;
    LogicalDevice &device;
    LogicalQueue &presentQueue;
    int currentFrame = 0;
    vk::SwapchainKHR swapchains[1];
    vk::PipelineStageFlags waitStages[1];

public:
    unsigned int prepareForNextImage(SwapChain &swapChain) {
        unsigned int result = 0;
        device.getDevice().waitForFences(1,
                                         &inFlightFences[currentFrame],
                                         VK_TRUE,
                                         std::numeric_limits<uint64_t>::max());
        device.getDevice().acquireNextImageKHR(swapChain.getSwapchainKhr(), std::numeric_limits<uint64_t>::max(),
                                               imageAvailableSemaphores[currentFrame],
                                               VK_NULL_HANDLE,
                                               &result);


        return result;
    }

    void submitCommandBuffers(vk::CommandBuffer *buffers, SwapChain &swapChain, uint32_t *currentImage) {
        if (imagesInFlight[*currentImage] != VK_NULL_HANDLE) {
            device.getDevice().waitForFences(1,
                                             &imagesInFlight[*currentImage],
                                             VK_TRUE,
                                             UINT64_MAX);
        }
        imagesInFlight[*currentImage] = inFlightFences[currentFrame];

        vk::SubmitInfo submitInfo = {};

        waitStages[0] = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;

        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

        device.getDevice().resetFences(1, &inFlightFences[currentFrame]);

        presentQueue.getQueue().submit(1, &submitInfo, inFlightFences[currentFrame]);

        vk::PresentInfoKHR presentInfo = {};
        swapchains[0] = swapChain.getSwapchainKhr();
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;

        presentInfo.pImageIndices = currentImage;

        auto result = presentQueue.getQueue().presentKHR(presentInfo);

        currentFrame = (currentFrame + 1) % maxFramesInFlight;
    }

private:
    void createSyncObjects() {
        imageAvailableSemaphores.resize(maxFramesInFlight);
        renderFinishedSemaphores.resize(maxFramesInFlight);
        inFlightFences.resize(maxFramesInFlight);
        imagesInFlight.resize(3);
        vk::SemaphoreCreateInfo semaphoreInfo = {};
        vk::FenceCreateInfo fenceInfo = {};
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
        for (size_t i = 0; i < maxFramesInFlight; i++) {
            imageAvailableSemaphores[i] = device.getDevice().createSemaphore(semaphoreInfo);
            renderFinishedSemaphores[i] = device.getDevice().createSemaphore(semaphoreInfo);
            inFlightFences[i] = device.getDevice().createFence(fenceInfo);
            imagesInFlight[i] = nullptr;
        }
    }

    void destroy() override {
        device.getDevice().waitIdle();
        for (uint32_t i = 0; i < maxFramesInFlight; ++i) {
            device.getDevice().destroySemaphore(imageAvailableSemaphores[i]);
            device.getDevice().destroySemaphore(renderFinishedSemaphores[i]);
            device.getDevice().destroyFence(inFlightFences[i]);
        }
        destroyed = true;
    }
};


