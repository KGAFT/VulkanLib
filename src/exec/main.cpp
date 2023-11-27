
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
#include "RenderPipeline/FrameBufferManager.hpp"
#include "VulkanLib/Device/Buffer/Buffer.hpp"

class ResizeCallback : public IWindowResizeCallback{
public:
    ResizeCallback(RenderPipeline *renderPipeline, FrameBufferManager *frameBufferManager, SyncManager *syncManager,
                   const std::shared_ptr<LogicalDevice> &device) : renderPipeline(renderPipeline),
                                                                   frameBufferManager(frameBufferManager),
                                                                   syncManager(syncManager), device(device) {}

private:
    RenderPipeline* renderPipeline;
    FrameBufferManager* frameBufferManager;
    SyncManager* syncManager;
    std::shared_ptr<LogicalDevice> device;
public:
    void resized(int width, int height) override {
        device->getDevice().waitIdle();
        syncManager->setStop(true);
        renderPipeline->recreateForResize(width, height);
        frameBufferManager->resize(width, height);
        syncManager->setStop(false);
    }
};

void bufferTest(std::shared_ptr<LogicalDevice> device);

int main() {


    glfwInit();
    std::vector<Monitor*> monitors;
    Monitor::enumerateMonitors(monitors);
    Window *window = Window::createWindow(640, 480, "Vulkan test app", nullptr, false);
    window->enableRefreshRateInfo();
    InstanceBuilder builder;
    builder.presetForGlfw();
    builder.presetForDebug();
    builder.setApplicationName("TestEngine");
    Instance instance(builder);
    auto devices = PhysicalDevice::getDevices(
            instance);
    VkSurfaceKHR surfaceKhr = window->getWindowSurface(instance.getInstance());
    DeviceBuilder devBuilder;
    devBuilder.addExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    devBuilder.requestGraphicsSupport();
    devBuilder.requestPresentSupport(surfaceKhr);

    std::vector<DeviceSuitabilityResults> results;

    for (auto &el: *devices) {
        results.push_back(DeviceSuitabilityResults());
        if (DeviceSuitability::isDeviceSuitable(devBuilder, el, &results[results.size() - 1])) {
            std::cout << el->properties.deviceName << std::endl;
        }
    }
    int devIndex=0;
    std::cin >> devIndex;
    std::shared_ptr<LogicalDevice> device = std::make_shared<LogicalDevice>
            (instance, (*devices)[devIndex], devBuilder, &results[devIndex]);


    devices->clear();
    results.clear();
    bufferTest(device);
std::cin>>devIndex;

    std::shared_ptr<SwapChain> swapChain(
            new SwapChain(device, surfaceKhr, window->getWidth(), window->getHeight(), false));


    auto *loaderInstance = ShaderLoader::getInstance();
    std::vector<ShaderCreateInfo> createInfos;
    createInfos.push_back(
            {"shaders/main.vert", "main.vert", ShaderFileType::SRC_FILE, vk::ShaderStageFlagBits::eVertex});
    createInfos.push_back(
            {"shaders/main.frag", "main.frag", ShaderFileType::SRC_FILE, vk::ShaderStageFlagBits::eFragment});

    Shader *shader = loaderInstance->createShader(*device, createInfos);

    FrameBufferManager frameBufferManager(device, swapChain, swapChain->getSwapchainImageViews().size(),
                                          window->getWidth(), window->getHeight());

    GraphicsPipelineBuilder *gPipelineBuilder = GraphicsPipelineBuilder::getInstance();
    gPipelineBuilder->addColorAttachments(
            const_cast<std::shared_ptr<ImageView> *>(swapChain->getSwapchainImageViews().data()),
            swapChain->getSwapchainImageViews().size());
    gPipelineBuilder->addDepthAttachments(frameBufferManager.getDepthAttachments(), 1);


    RenderPipelineBuilder rpBuilder;
    rpBuilder.setGraphicsPipelineBuilder(gPipelineBuilder);
    rpBuilder.setShader(shader);
    rpBuilder.setImagePerStepAmount(1);
    rpBuilder.setWidth(window->getWidth());
    rpBuilder.setHeight(window->getHeight());
    rpBuilder.setSwapChain(swapChain.get());

    RenderPipeline renderPipeline(*device, rpBuilder);

    frameBufferManager.initializeFrameBuffers(renderPipeline.getRenderPass());

    renderPipeline.setClearColorValues(0.0f, 0.0f, 0.0f, 1.0f);

    SyncManager syncManager(*device, *swapChain, *device->getPresentQueue(),
                            swapChain->getSwapchainImageViews().size());

    window->addResizeCallback(new ResizeCallback(&renderPipeline, &frameBufferManager, &syncManager, device));

    while (!window->needToClose()) {
        try {
            window->preRenderEvents();
            if (!syncManager.isStop()) {
                uint32_t currentImage;
                vk::CommandBuffer commandBuffer = syncManager.beginRender(currentImage);
                renderPipeline.beginRender(commandBuffer, *frameBufferManager.getFrameBuffer(currentImage));

                commandBuffer.draw(3, 1, 0, 0);
                renderPipeline.endRender(commandBuffer);
                syncManager.endRender();
                window->postRenderEvents();
            }

        } catch (std::exception &exception) {

        }

    }
    return 0;
}

void bufferTest(std::shared_ptr<LogicalDevice> device){
    std::string testText = "Hello world. \n TestMessage";

    uint32_t indices[] = {device->getQueueByType(vk::QueueFlagBits::eGraphics)->getIndex()};

    vk::BufferCreateInfo stageCreateInfo{};
    stageCreateInfo.size = testText.length()*sizeof(char);
    stageCreateInfo.pQueueFamilyIndices = indices;
    stageCreateInfo.queueFamilyIndexCount = 1;
    stageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
    stageCreateInfo.usage = vk::BufferUsageFlagBits::eTransferSrc|vk::BufferUsageFlagBits::eTransferDst|vk::BufferUsageFlagBits::eStorageBuffer;

    void* mapPoint;

    Buffer staging(device, stageCreateInfo, vk::MemoryPropertyFlagBits::eHostVisible);
    staging.map(&mapPoint,0, vk::MemoryMapFlags());
    memcpy(mapPoint, testText.c_str(), stageCreateInfo.size);
    staging.unMap();

    Buffer deviceLocal(device, stageCreateInfo, vk::MemoryPropertyFlagBits::eDeviceLocal);
    vk::CommandBuffer sgtCommands = device->getQueueByType(vk::QueueFlagBits::eGraphics)->beginSingleTimeCommands();
    deviceLocal.copyFromBuffer(sgtCommands, staging, stageCreateInfo.size, 0, 0);
    device->getQueueByType(vk::QueueFlagBits::eGraphics)->endSingleTimeCommands(sgtCommands);

    staging.map(&mapPoint, 0, vk::MemoryMapFlags());
    for(size_t i = 0; i<testText.length(); i+=1){
        ((char*)mapPoint)[i] = 0;
    }
    staging.unMap();

    sgtCommands = device->getQueueByType(vk::QueueFlagBits::eGraphics)->beginSingleTimeCommands();
    staging.copyFromBuffer(sgtCommands, deviceLocal, stageCreateInfo.size, 0, 0);
    device->getQueueByType(vk::QueueFlagBits::eGraphics)->endSingleTimeCommands(sgtCommands);

    staging.map(&mapPoint, 0, vk::MemoryMapFlags());
    for(size_t i = 0; i<testText.length(); i+=1){
        std::cout<<((char*)mapPoint)[i];
    }
}
