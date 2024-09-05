//
// Created by kgaft on 2/18/24.
//

#ifndef VULKANRENDERENGINE_DEFAULTVULKANFILELOGGERCALLBACK_HPP
#define VULKANRENDERENGINE_DEFAULTVULKANFILELOGGERCALLBACK_HPP

#include <chrono>
#include "IInstanceLoggerCallback.hpp"
#include <fstream>
#include <iomanip>

class DefaultVulkanFileLoggerCallback : public IInstanceLoggerCallback
{
public:
    DefaultVulkanFileLoggerCallback(const char* whereToPutLog): filePath(whereToPutLog){
        logFile.open(whereToPutLog);
    }
private:
    std::ofstream logFile;
    const char* filePath;
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
        outputTime[outputTime.size() - 1] = ' ';
        std::string outputMessage = outputTime + "VULKAN" + " [" + severity + "] " + type + " " + message;
        std::string severityS = severity;
        logFile<<outputMessage<<std::endl;
        logFile.flush();
    }
};


#endif //VULKANRENDERENGINE_DEFAULTVULKANFILELOGGERCALLBACK_HPP
