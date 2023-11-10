
#include "VulkanLib/Instance.hpp"
#include <GLFW/glfw3.h>
#include "VulkanLib/Device/PhysicalDevice/PhysicalDevice.hpp"
#include "VulkanLib/Device/DeviceBuilder.hpp"
#include "VulkanLib/Device/PhysicalDevice/DeviceSuitability.hpp"
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "VulkanLib/MemoryUtils/VectorUtils.hpp"
#include "VulkanLib/Device/SwapChain/SwapChain.hpp"
#include "VulkanLib/Shader/ShaderLoader.hpp"

int main() {


    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(800, 600, "tes", nullptr, nullptr);

    InstanceBuilder builder;
    builder.presetForGlfw();
    builder.presetForDebug();
    builder.setApplicationName("TestEngine");
    Instance instance(builder);
    std::vector<PhysicalDevice *> devices = PhysicalDevice::getDevices(instance);
    VkSurfaceKHR surfaceKhr;
    glfwCreateWindowSurface(instance.getInstance(), window, nullptr, &surfaceKhr);
    DeviceBuilder devBuilder;
    devBuilder.addExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    devBuilder.requestGraphicsSupport();
    devBuilder.requestPresentSupport(surfaceKhr);

    std::vector<DeviceSuitabilityResults> results;

    for(auto& el : devices){
        results.push_back(DeviceSuitabilityResults());
        if(DeviceSuitability::isDeviceSuitable(devBuilder, el, &results[results.size()-1])){
            std::cout<<el->properties.deviceName<<std::endl;
        }
    }
    int devIndex;
    std::cin>>devIndex;
    LogicalDevice device(instance, devices[devIndex], devBuilder, &results[devIndex]);
    SwapChain swapChain(device, surfaceKhr, 800, 600);
    auto* loaderInstance = ShaderLoader::getInstance();
    size_t shaderSize;
    std::vector<uint32_t> shader;
    loaderInstance->compileShader("main.frag", "main.frag", shaderc_fragment_shader, shader);
    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
    }
    return 0;
}
