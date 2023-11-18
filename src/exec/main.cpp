
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

struct PCS {
    glm::vec4 colorAmplifier;
    glm::vec4 colorAdder;
};

int main() {


    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow *window = glfwCreateWindow(800, 600, "tes", nullptr, nullptr);

    InstanceBuilder builder;
    builder.presetForGlfw();
    builder.presetForDebug();
    builder.setApplicationName("TestEngine");
    Instance instance(builder);
    std::vector<PhysicalDevice *> &devices = const_cast<std::vector<PhysicalDevice *> &>(PhysicalDevice::getDevices(
            instance));
    VkSurfaceKHR surfaceKhr;
    glfwCreateWindowSurface(instance.getInstance(), window, nullptr, &surfaceKhr);
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


    SwapChain swapChain(device, surfaceKhr, 800, 600);
    std::vector<Image> depthImages;
    depthImages.resize(swapChain.getSwapchainImageViews().size());
    vk::Format depthFormat = device.findDepthFormat();

    vk::ImageCreateInfo imageInfo{};
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent.width = 800;
    imageInfo.extent.height = 600;
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

    GraphicsPipelineBuilder gPipelineBuilder;
    gPipelineBuilder.addColorAttachments(const_cast<ImageView **>(swapChain.getSwapchainImageViews().data()),
                                         swapChain.getSwapchainImageViews().size());
    gPipelineBuilder.addDepthAttachments(depthImageViews.data(), depthImageViews.size());
    gPipelineBuilder.addVertexInput({0, 3, sizeof(float), vk::Format::eR32G32B32Sfloat});
    gPipelineBuilder.addVertexInput({1, 2, sizeof(float), vk::Format::eR32G32Sfloat});
    gPipelineBuilder.addSamplerInfo({1, vk::ShaderStageFlagBits::eFragment});
    gPipelineBuilder.addPushConstantInfo({vk::ShaderStageFlagBits::eFragment, sizeof(PCS)});

    RenderPass renderPass(true, 1, gPipelineBuilder, device);
    std::vector<FrameBuffer> frameBuffers;
    std::vector<ImageView*> attachments;
    uint32_t c = 0;
    for (auto &item: swapChain.getSwapchainImageViews()){
        attachments.push_back(item);
        attachments.push_back(depthImageViews[c]);
        c++;
    }

    for (int i = 0; i < attachments.size(); i+=2){
        frameBuffers.push_back(FrameBuffer(device, renderPass.getRenderPass(), &attachments[i], 2, 800, 600));
    }

    GraphicsPipeline pipeline(device, shader, gPipelineBuilder, 1, 800, 600, renderPass);

    SyncManager syncManager(device, swapChain, device.getPresentQueue());


    while (!glfwWindowShouldClose(window)) {
        uint32_t currentImage;
        vk::CommandBuffer commandBuffer = syncManager.beginRender(currentImage);
        renderPass.begin(commandBuffer, frameBuffers[currentImage], 800, 600, 1);
        renderPass.end(commandBuffer);

        syncManager.endRender();
        glfwPollEvents();
    }
    return 0;
}
