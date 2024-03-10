//
// Created by kgaft on 3/11/24.
//
#include "InstanceBuilder.hpp"

void InstanceBuilder::addLayer(const char *layer) {
    layers.push_back(layer);
}

void InstanceBuilder::addExtension(const char *extension) {
    extensions.push_back(extension);
}

void InstanceBuilder::addExtensions(const char **ppExtensions, unsigned int extensionCount) {
    for (unsigned int i = 0; i < extensionCount; ++i) {
        InstanceBuilder::extensions.push_back(ppExtensions[i]);
    }
}

void InstanceBuilder::addLayers(const char **ppLayers, unsigned int layersCount) {
    for (unsigned int i = 0; i < layersCount; ++i) {
        InstanceBuilder::layers.push_back(ppLayers[i]);
    }
}

void InstanceBuilder::presetForDebug() {
    layers.push_back("VK_LAYER_KHRONOS_validation");
    extensions.push_back("VK_EXT_debug_utils");
    debugEnabled = true;
}

void InstanceBuilder::setSaveDefaultVulkanLoggerCallback(bool isSaveDefaultVulkanLoggerCallback) {
    InstanceBuilder::saveDefaultVulkanLoggerCallback = isSaveDefaultVulkanLoggerCallback;
}

void InstanceBuilder::setApplicationName(const char *pApplicationName) {
    InstanceBuilder::applicationName = pApplicationName;
}

void InstanceBuilder::addLoggerCallback(IInstanceLoggerCallback *instanceLoggerCallback) {
    startLoggerCallbacks.push_back(instanceLoggerCallback);
}
