//
// Created by kgaft on 11/4/23.
//
#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <GLFW/glfw3.h>

class InstanceBuilder {
    friend class Instance;

public:
    InstanceBuilder() {}

private:
    std::vector<const char *> layers;
    std::vector<const char *> extensions{"VK_KHR_get_physical_device_properties2"};
    bool debugEnabled = false;
    const char *applicationName = nullptr;
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

    void presetForDebug() {
        layers.push_back("VK_LAYER_KHRONOS_validation");
        extensions.push_back("VK_EXT_debug_utils");
        debugEnabled = true;
    }

    void presetForGlfw() {
        uint32_t extCount;
        const char **extensions = glfwGetRequiredInstanceExtensions(&extCount);
        addExtensions(extensions, extCount);
    }

    void setApplicationName(const char *applicationName) {
        InstanceBuilder::applicationName = applicationName;
    }
};

