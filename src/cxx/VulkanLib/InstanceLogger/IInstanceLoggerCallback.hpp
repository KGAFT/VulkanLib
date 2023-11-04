//
// Created by kgaft on 11/4/23.
//
#pragma once

enum LoggerCallbackType {
    RAW_VULKAN_DEFS = 1,
    TRANSLATED_DEFS = 2,
    BOTH_DEFS = 3
};


#include <vulkan/vulkan.h>
#include <string>


class IInstanceLoggerCallback {
public:
    virtual LoggerCallbackType getCallBackMode() = 0;

    virtual void messageRaw(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type,
                            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) = 0;

    virtual void translatedMessage(const char *severity, const char *type, std::string& message) = 0;
};