//
// Created by kgaft on 11/4/23.
//
#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <GLFW/glfw3.h>
#include <SDL.h>
#include <SDL_vulkan.h>
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
        layers.push_back("VK_LAYER_LUNARG_api_dump");
        extensions.push_back("VK_EXT_debug_utils");
        debugEnabled = true;
    }

    void setSaveDefaultVulkanLoggerCallback(bool saveDefaultVulkanLoggerCallback) {
        InstanceBuilder::saveDefaultVulkanLoggerCallback = saveDefaultVulkanLoggerCallback;
    }

    void presetForGlfw() {
        uint32_t extCount;
        const char **ppExtensions = glfwGetRequiredInstanceExtensions(&extCount);
        addExtensions(ppExtensions, extCount);
    }
    void presetForSDL3(){
        uint32_t extensionCount;
        char const* const* ppExtensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
        for (uint32_t i = 0; i < extensionCount; ++i){
            addExtension(ppExtensions[i]);
        }
    }

    void setApplicationName(const char *pApplicationName) {
        InstanceBuilder::applicationName = pApplicationName;
    }
};

