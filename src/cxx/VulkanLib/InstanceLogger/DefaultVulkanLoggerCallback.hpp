//
// Created by kgaft on 11/4/23.
//
#pragma once

#include <chrono>
#include "IInstanceLoggerCallback.hpp"
#include <iostream>

class DefaultVulkanLoggerCallback : public IInstanceLoggerCallback
{
public:
    LoggerCallbackType getCallBackMode() override{
        return TRANSLATED_DEFS;
    }

    void messageRaw(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type,
                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) override{

    }

    void translatedMessage(const char *severity, const char *type, std::string& message) override{
        auto currentTime = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(currentTime);
        std::string outputTime = std::string(ctime(&time));
        outputTime[outputTime.size() - 1] = '\0';
        std::string outputMessage = outputTime + " VULKAN" + " [" + severity + "] " + type + " " + message;
        std::string severityS = severity;
        if (!severityS.compare("ERROR"))
        {
            std::cerr << outputMessage << std::endl;
        }
        else
        {
            std::cout << outputMessage << std::endl;
        }
        std::cout.flush();
    }
};