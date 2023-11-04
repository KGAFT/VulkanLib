#include <iostream>

#include "VulkanLib/Instance.hpp"
#include <GLFW/glfw3.h>
int main() {
    glfwInit();
    InstanceBuilder builder;
    builder.presetForGlfw();
    builder.presetForDebug();
    Instance instance(builder);

    return 0;
}
