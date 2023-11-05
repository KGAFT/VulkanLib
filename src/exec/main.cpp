
#include "VulkanLib/Instance.hpp"
#include <GLFW/glfw3.h>
#include "VulkanLib/Device/LogicalDevice/PhysicalDevice.hpp"
#include "VulkanLib/Device/DeviceBuilder.hpp"
#include "VulkanLib/Device/LogicalDevice/DeviceSuitability.hpp"

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

    for(auto& el : devices){
        if(DeviceSuitability::isDeviceSuitable(devBuilder, el)){
            std::cout<<el->properties.deviceName<<std::endl;
        }
    }

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
    }
    return 0;
}
