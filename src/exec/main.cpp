#include <iostream>

#include "VulkanLib/Instance.hpp"
#include <GLFW/glfw3.h>
int main() {
    glfwInit();
    VulkanLib::InstanceBuilder builder;
    builder.presetForGlfw();
    VulkanLib::Instance instance(&builder);
    return 0;
}
