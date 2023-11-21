
#include "VulkanLib/Instance.hpp"
#include <GLFW/glfw3.h>
#include "VulkanLib/Device/PhysicalDevice/PhysicalDevice.hpp"
#include "VulkanLib/Device/DeviceBuilder.hpp"
#include "VulkanLib/Device/PhysicalDevice/DeviceSuitability.hpp"
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "VulkanLib/MemoryUtils/VectorUtils.hpp"
#include "VulkanLib/Device/SwapChain/SwapChain.hpp"
#include "VulkanLib/Shader/ShaderLoader.hpp"
#include "VulkanLib/GraphicsPipeline/Configuration/GraphicsPipelineBuilder.hpp"
#include "VulkanLib/GraphicsPipeline/RenderPass/RenderPass.hpp"
#include "VulkanLib/GraphicsPipeline/GraphicsPipeline.hpp"
#include "VulkanLib/Device/Synchronization/SyncManager.hpp"
#include "VulkanLib/GraphicsPipeline/RenderPass/FrameBuffer.hpp"
#include <glm/glm.hpp>
#include "Window/Window.hpp"
#include "RenderPipeline/RenderPipelineBuilder.hpp"
#include "RenderPipeline/RenderPipeline.hpp"

class ResizeCallback : public IWindowResizeCallback {
public:
    ResizeCallback(RenderPipeline *renderPipeline, std::vector<FrameBuffer> &frameBuffers,
                   std::vector<Image> &depthImages, SwapChain *swapChain, LogicalDevice &device, SyncManager& syncManager) : renderPipeline(
            renderPipeline), frameBuffers(frameBuffers), depthImages(depthImages), swapChain(swapChain),
                                                                                                   device(device), syncManager(syncManager) {}

private:
    RenderPipeline *renderPipeline;
    std::vector<FrameBuffer> &frameBuffers;
    std::vector<Image> &depthImages;
    SwapChain *swapChain;
    LogicalDevice &device;
    SyncManager& syncManager;
public:
    void resized(int width, int height) override {
        device.getDevice().waitIdle();
        syncManager.setStop(true);
        swapChain->recreate(width, height, true);
        renderPipeline->recreateForResize(width, height);
        std::vector<ImageView *> attachments;
        for (auto &item: depthImages) {
            item.resize(width, height);
        }
        uint32_t c = 0;
        for (auto &item: swapChain->getSwapchainImageViews()) {
            attachments.push_back(item);
            attachments.push_back(&depthImages[c].getImageViews()[0]);
            c++;
        }
        c = 0;
        for (auto &item: frameBuffers) {
            item.recreate(&attachments[c], 2, width, height);
            c += 2;
        }
        syncManager.setStop(false);
    }
};

int main() {


    glfwInit();
    Window *window = Window::createWindow(640, 480, "Vulkan test app", nullptr, false);
    window->enableRefreshRateInfo();
    InstanceBuilder builder;
    builder.presetForGlfw();
    builder.presetForDebug();
    builder.setApplicationName("TestEngine");
    Instance instance(builder);
    std::vector<PhysicalDevice *> &devices = const_cast<std::vector<PhysicalDevice *> &>(PhysicalDevice::getDevices(
            instance));
    VkSurfaceKHR surfaceKhr = window->getWindowSurface(instance.getInstance());
    DeviceBuilder devBuilder;
    devBuilder.addExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    devBuilder.requestGraphicsSupport();
    devBuilder.requestPresentSupport(surfaceKhr);

    std::vector<DeviceSuitabilityResults> results;

    for (auto &el: devices) {
        results.push_back(DeviceSuitabilityResults());
        if (DeviceSuitability::isDeviceSuitable(devBuilder, el, &results[results.size() - 1])) {
            std::cout << el->properties.deviceName << std::endl;
        }
    }
    int devIndex;
    std::cin >> devIndex;
    LogicalDevice device(instance, devices[devIndex], devBuilder, &results[devIndex]);

    PhysicalDevice::releaseUnusedDevicesInfos(&devices[devIndex], 1);
    results.clear();


    SwapChain swapChain(device, surfaceKhr, window->getWidth(), window->getHeight(), false);
    std::vector<Image> depthImages;
    depthImages.resize(swapChain.getSwapchainImageViews().size());
    vk::Format depthFormat = device.findDepthFormat();

    vk::ImageCreateInfo imageInfo{};
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent.width = window->getWidth();
    imageInfo.extent.height = window->getHeight();
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = depthFormat;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;
    imageInfo.flags = vk::ImageCreateFlags();

    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = depthFormat;
    viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    std::vector<ImageView *> depthImageViews;

    for (unsigned int i = 0; i < swapChain.getSwapchainImageViews().size(); ++i) {
        depthImages[i].initialize(&device, imageInfo);
        viewInfo.image = depthImages[i].getBase();
        depthImageViews.push_back(&depthImages[i].createImageView(viewInfo));
    }

    auto *loaderInstance = ShaderLoader::getInstance();
    std::vector<ShaderCreateInfo> createInfos;
    createInfos.push_back(
            {"shaders/main.vert", "main.vert", ShaderFileType::SRC_FILE, vk::ShaderStageFlagBits::eVertex});
    createInfos.push_back(
            {"shaders/main.frag", "main.frag", ShaderFileType::SRC_FILE, vk::ShaderStageFlagBits::eFragment});

    Shader *shader = loaderInstance->createShader(device, createInfos);

    GraphicsPipelineBuilder *gPipelineBuilder = GraphicsPipelineBuilder::getInstance();
    gPipelineBuilder->addColorAttachments(const_cast<ImageView **>(swapChain.getSwapchainImageViews().data()),
                                          swapChain.getSwapchainImageViews().size());
    gPipelineBuilder->addDepthAttachments(depthImageViews.data(), depthImageViews.size());


    RenderPipelineBuilder rpBuilder;
    rpBuilder.setGraphicsPipelineBuilder(gPipelineBuilder);
    rpBuilder.setShader(shader);
    rpBuilder.setImagePerStepAmount(1);
    rpBuilder.setWidth(window->getWidth());
    rpBuilder.setHeight(window->getHeight());
    rpBuilder.setSwapChain(&swapChain);

    RenderPipeline renderPipeline(device, rpBuilder);

    std::vector<FrameBuffer> frameBuffers;
    std::vector<ImageView *> attachments;
    uint32_t c = 0;
    for (auto &item: swapChain.getSwapchainImageViews()) {
        attachments.push_back(item);
        attachments.push_back(depthImageViews[c]);
        c++;
    }

    for (int i = 0; i < attachments.size(); i += 2) {
        frameBuffers.push_back(FrameBuffer(device, renderPipeline.getRenderPass()->getRenderPass(), &attachments[i], 2,
                                           window->getWidth(), window->getHeight()));
    }


    renderPipeline.setClearColorValues(0, 0, 0, 1);

    SyncManager syncManager(device, swapChain, device.getPresentQueue(), frameBuffers.size());
    window->addResizeCallback(new ResizeCallback(&renderPipeline, frameBuffers, depthImages, &swapChain, device, syncManager));
    while (!window->needToClose()) {
        try{
            window->preRenderEvents();
            if(!syncManager.isStop()){
                uint32_t currentImage;
                vk::CommandBuffer commandBuffer = syncManager.beginRender(currentImage);
                renderPipeline.beginRender(commandBuffer, frameBuffers[currentImage]);

                //commandBuffer.draw(3, 1, 0, 0);
                renderPipeline.endRender(commandBuffer);
                syncManager.endRender();
                window->postRenderEvents();
            }

        }catch(std::exception& exception){

        }

    }
    return 0;
}
