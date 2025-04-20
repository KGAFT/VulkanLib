//
// Created by kgaft on 11/4/23.
//
#ifndef VULKANLIB_INSTANCEBUILDER_HPP
#define VULKANLIB_INSTANCEBUILDER_HPP
#include <vulkan/vulkan.h>
#include <vector>

#include "VulkanLib/InstanceLogger/IInstanceLoggerCallback.hpp"

class InstanceBuilder {
    friend class Instance;

public:
    InstanceBuilder() = default;

private:
    std::vector<const char *> layers;
    std::vector<const char *> extensions{"VK_KHR_get_physical_device_properties2", "VK_EXT_swapchain_colorspace"};
    std::vector<IInstanceLoggerCallback *> startLoggerCallbacks;
    bool debugEnabled = false;
    const char *applicationName = nullptr;
    bool saveDefaultVulkanLoggerCallback = false;
public:
    void addLayer(const char *layer);

    void addExtension(const char *extension);

    void addExtensions(const char **ppExtensions, unsigned int extensionCount);

    void addLayers(const char **ppLayers, unsigned int layersCount);

    void addLoggerCallback(IInstanceLoggerCallback *instanceLoggerCallback);

    void presetForDebug();

    void setSaveDefaultVulkanLoggerCallback(bool isSaveDefaultVulkanLoggerCallback);

    void setApplicationName(const char *pApplicationName);
};

#endif
