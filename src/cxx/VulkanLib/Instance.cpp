//
// Created by kgaft on 3/11/24.
//
#include "Instance.hpp"

bool Instance::debugSupported() {
    for (auto &item: vk::enumerateInstanceLayerProperties()) {
        if (!strcmp(item.layerName, "VK_LAYER_KHRONOS_validation")) {
            return true;
        }
    }
    return false;
}

Instance::Instance(InstanceBuilder &pBuilder) {
    vk::ApplicationInfo appInfo(
            pBuilder.applicationName,
            VK_MAKE_VERSION(1, 0, 0),
            "VulkanLib",
            VK_MAKE_VERSION(1, 0, 0),
            VK_API_VERSION_1_2
    );
    vk::InstanceCreateInfo createInfo(
            vk::InstanceCreateFlags(),
            &appInfo,
            pBuilder.layers.size(), pBuilder.layers.data(),
            pBuilder.extensions.size(), pBuilder.extensions.data()
    );
    try {
        instance = vk::createInstance(createInfo, nullptr);
        dynamicLoader = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
        if (pBuilder.debugEnabled) {
            logger = new InstanceLogger(instance, dynamicLoader, pBuilder.startLoggerCallbacks,
                                        pBuilder.saveDefaultVulkanLoggerCallback);
            enabledLayers = pBuilder.layers;
        }
    } catch (vk::SystemError &error) {
        std::cerr << error.what() << std::endl;
    }
}

InstanceLogger *Instance::getLogger() const {
    return logger;
}

vk::Instance &Instance::getInstance() {
    return instance;
}

const std::vector<const char *> &Instance::getEnabledLayers() const {
    return enabledLayers;
}

vk::DispatchLoaderDynamic &Instance::getDynamicLoader() {
    return dynamicLoader;
}

void Instance::destroy() {
    delete logger;
    instance.destroy();
    destroyed = true;
}
