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

        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
        auto outputTime = oss.str();
        std::string outputMessage = outputTime + "VULKAN" + " [" + severity + "] " + type + " " + message;
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