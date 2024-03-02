//
// Created by kgaft on 11/4/23.
//
#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "VulkanLib/InstanceLogger/IInstanceLoggerCallback.hpp"

class InstanceBuilder {
    friend class Instance;

public:
    InstanceBuilder() {}

private:
    std::vector<const char *> layers;
    std::vector<const char *> extensions{"VK_KHR_get_physical_device_properties2"};
    std::vector<IInstanceLoggerCallback*> startLoggerCallbacks;
    bool debugEnabled = false;
    const char *applicationName = nullptr;
    bool saveDefaultVulkanLoggerCallback = false;
public:
    void addLayer(const char *layer) {
        layers.push_back(layer);
    }

    void addExtension(const char *extension) {
        extensions.push_back(extension);
    }

    void addExtensions(const char **extensions, unsigned int extensionCount) {
        for (unsigned int i = 0; i < extensionCount; ++i) {
            InstanceBuilder::extensions.push_back(extensions[i]);
        }
    }

    void addLayers(const char **layers, unsigned int layersCount) {
        for (unsigned int i = 0; i < layersCount; ++i) {
            InstanceBuilder::layers.push_back(layers[i]);
        }
    }
    void addLoggerCallback(IInstanceLoggerCallback* instanceLoggerCallback){
        startLoggerCallbacks.push_back(instanceLoggerCallback);
    }

    void presetForDebug() {
        layers.push_back("VK_LAYER_KHRONOS_validation");
        extensions.push_back("VK_EXT_debug_utils");
        debugEnabled = true;
    }

    void setSaveDefaultVulkanLoggerCallback(bool saveDefaultVulkanLoggerCallback) {
        InstanceBuilder::saveDefaultVulkanLoggerCallback = saveDefaultVulkanLoggerCallback;
    }


    void setApplicationName(const char *pApplicationName) {
        InstanceBuilder::applicationName = pApplicationName;
    }
};

