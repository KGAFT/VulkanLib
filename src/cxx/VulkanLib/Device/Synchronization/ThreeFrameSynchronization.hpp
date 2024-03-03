//
// Created by kgaft on 11/18/23.
//
#pragma once

#include <vector>
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "VulkanLib/Device/SwapChain/SwapChain.hpp"


class ThreeFrameSynchronization : public IDestroyableObject {
public:
    ThreeFrameSynchronization(std::shared_ptr<LogicalDevice> device, std::shared_ptr<LogicalQueue> graphicsQueue,
                              uint32_t maxFramesInFlight) : maxFramesInFlight(maxFramesInFlight), device(device),
                                                            presentQueue(graphicsQueue) {
        createSyncObjects();
    }

private:
    uint32_t maxFramesInFlight;
    std::vector<vk::Semaphore> imageAvailableSemaphores;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    std::vector<vk::Fence> inFlightFences;
    std::vector<vk::Fence> imagesInFlight;
    std::shared_ptr<LogicalDevice> device;
    std::shared_ptr<LogicalQueue> presentQueue;
    int currentFrame = 0;
    vk::SwapchainKHR swapchains[1];
    vk::PipelineStageFlags waitStages[1];

public:
    unsigned int prepareForNextImage(std::shared_ptr<SwapChain> swapChain) {
        vk::Result res;
        unsigned int result = 0;
        res = device->getDevice().waitForFences(1,
                                               &inFlightFences[currentFrame],
                                               VK_TRUE,
                                               std::numeric_limits<uint64_t>::max());
        if(res!=vk::Result::eSuccess){
            std::cerr<<"Failed to wait for fences"<<std::endl;
        }
        res = device->getDevice().acquireNextImageKHR(swapChain->getSwapchainKhr(), std::numeric_limits<uint64_t>::max(),
                                                     imageAvailableSemaphores[currentFrame],
                                                     vk::Fence(),
                                                     &result);

        if(res!=vk::Result::eSuccess){
            std::cerr<<"Failed to wait for fences"<<std::endl;
        }
        return result;
    }

    void submitCommandBuffers(vk::CommandBuffer *buffers, std::shared_ptr<SwapChain> swapChain, uint32_t *currentImage) {
        vk::Result res;
        if (imagesInFlight[*currentImage] != VK_NULL_HANDLE) {
            res = device->getDevice().waitForFences(1,
                                                   &imagesInFlight[*currentImage],
                                                   VK_TRUE,
                                                   UINT64_MAX);
            if(res!=vk::Result::eSuccess){
                std::cerr<<"Failed to wait for fences"<<std::endl;
            }
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

        res = device->getDevice().resetFences(1, &inFlightFences[currentFrame]);
        if(res!=vk::Result::eSuccess){
            std::cerr<<"Failed to reset fences"<<std::endl;
        }

        res = presentQueue->getQueue().submit(1, &submitInfo, inFlightFences[currentFrame]);
        if(res!=vk::Result::eSuccess){
            std::cerr<<"Failed to submit queue"<<std::endl;
        }
        vk::PresentInfoKHR presentInfo = {};
        swapchains[0] = swapChain->getSwapchainKhr();
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;

        presentInfo.pImageIndices = currentImage;

        auto result = presentQueue->getQueue().presentKHR(presentInfo);
        if(result!=vk::Result::eSuccess){
            std::cerr<<"Failed to present frame"<<std::endl;
        }
        currentFrame = (currentFrame + 1) % maxFramesInFlight;
    }

    void waitStop() {
        for (auto &item: imagesInFlight) {
            if (item != VK_NULL_HANDLE) {
                vk::Result res;
                res = device->getDevice().waitForFences(1,
                                                       &item,
                                                       VK_TRUE,
                                                       UINT64_MAX);
                if(res!=vk::Result::eSuccess){
                    std::cerr<<"Failed to wait for fences"<<std::endl;
                }
            }
        }
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
            imageAvailableSemaphores[i] = device->getDevice().createSemaphore(semaphoreInfo);
            renderFinishedSemaphores[i] = device->getDevice().createSemaphore(semaphoreInfo);
            inFlightFences[i] = device->getDevice().createFence(fenceInfo);
            imagesInFlight[i] = nullptr;
        }
    }
public:
    void destroy() override {
        device->getDevice().waitIdle();
        for (uint32_t i = 0; i < maxFramesInFlight; ++i) {
            if(imageAvailableSemaphores[i]){
                device->getDevice().destroySemaphore(imageAvailableSemaphores[i]);
            }
            if(renderFinishedSemaphores[i]){
                device->getDevice().destroySemaphore(renderFinishedSemaphores[i]);
            }
            if(inFlightFences[i]){
                device->getDevice().destroyFence(inFlightFences[i]);
            }
        }
        imageAvailableSemaphores.clear();
        renderFinishedSemaphores.clear();
        inFlightFences.clear();
        destroyed = true;
    }
};


