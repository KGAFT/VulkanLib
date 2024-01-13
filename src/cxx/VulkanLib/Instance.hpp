//
// Created by kgaft on 11/4/23.
//
#pragma once

#include <vulkan/vulkan.hpp>
#include "InstanceBuilder.hpp"
#include "VulkanLib/InstanceLogger/InstanceLogger.hpp"
#include "VulkanLib/MemoryUtils/IDestroyableObject.hpp"

class Instance : IDestroyableObject{
public:
    static bool debugSupported(){
        for ( auto &item: vk::enumerateInstanceLayerProperties()){
            if(!strcmp(item.layerName, "VK_LAYER_KHRONOS_validation")){
                return true;
            }
        }
        return false;
    }

public:
    Instance(InstanceBuilder &pBuilder) {
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
                logger = new InstanceLogger(instance, dynamicLoader);
                enabledLayers = pBuilder.layers;
            }
        } catch (vk::SystemError &error) {
            std::cerr << error.what() << std::endl;
        }
    }

private:
    vk::Instance instance{nullptr};
    vk::DispatchLoaderDynamic dynamicLoader;
    std::vector<const char*> enabledLayers;
    InstanceLogger *logger = nullptr;
public:
    InstanceLogger *getLogger() const {
        return logger;
    }

    vk::Instance &getInstance() {
        return instance;
    }

    const std::vector<const char *> &getEnabledLayers() const {
        return enabledLayers;
    }

     vk::DispatchLoaderDynamic &getDynamicLoader() {
        return dynamicLoader;
    }

protected:
    void destroy() override {
        delete logger;
        instance.destroy();
        destroyed = true;
    }

};




