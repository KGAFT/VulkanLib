//
// Created by kgaft on 11/4/23.
//
#pragma once

#include "DefaultVulkanLoggerCallback.hpp"
#include "IInstanceLoggerCallback.hpp"
#include "VulkanLib/MemoryUtils/IDestroyableObject.hpp"
#include <cstddef>
#include <vulkan/vulkan.hpp>
#include <VulkanLib/VulkanLibAllocationCallback.h>

class InstanceLogger : public IDestroyableObject {
public:
  static inline std::vector<InstanceLogger *> loggers =
      std::vector<InstanceLogger *>();

  static VkBool32
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                void *pUserData) {
    for (auto item : loggers) {
      if (item == pUserData) {
        try {
          return item->debugCall(messageSeverity, messageType, pCallbackData);
        } catch (std::exception &exception) {
          std::cerr<<exception.what()<<std::endl;
          break;
        }
      }
    }
    return VK_FALSE;
  }

public:
  InstanceLogger(vk::Instance &instance,
                 vk::DispatchLoaderDynamic &dynamicLoader)
      : instance(instance), loaderDynamic(dynamicLoader) {
    vk::DebugUtilsMessengerCreateInfoEXT createInfo =
        vk::DebugUtilsMessengerCreateInfoEXT(
            vk::DebugUtilsMessengerCreateFlagsEXT(),
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding,
            debugCallback, this);
    loggers.push_back(this);
    messenger = instance.createDebugUtilsMessengerEXT(createInfo, VkLibAlloc::acquireAllocCb().get(),
                                                      dynamicLoader);
  }
  InstanceLogger(vk::Instance &instance,
                 vk::DispatchLoaderDynamic &dynamicLoader,
                 std::vector<IInstanceLoggerCallback *> &startCallbacks,
                 bool saveDefaultCallback)
      : instance(instance), loaderDynamic(dynamicLoader) {
    for (const auto &item : startCallbacks) {
      loggerCallbacks.push_back(item);
    }
    if (saveDefaultCallback)
      loggerCallbacks.push_back(new DefaultVulkanLoggerCallback);
    vk::DebugUtilsMessengerCreateInfoEXT createInfo =
        vk::DebugUtilsMessengerCreateInfoEXT(
            vk::DebugUtilsMessengerCreateFlagsEXT(),
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding,
            debugCallback, this);
    loggers.push_back(this);
    messenger = instance.createDebugUtilsMessengerEXT(createInfo, VkLibAlloc::acquireAllocCb().get(),
                                                      dynamicLoader);
  }

private:
  std::vector<IInstanceLoggerCallback *> loggerCallbacks;
  DefaultVulkanLoggerCallback defaultCallback;
  vk::DebugUtilsMessengerEXT messenger;
  vk::Instance &instance;
  vk::DispatchLoaderDynamic &loaderDynamic;

public:
  void addCallback(IInstanceLoggerCallback *loggerCallback) {
    loggerCallbacks.push_back(loggerCallback);
  }

private:
  VkBool32
  debugCall(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData) {
    const char *severityTranslated = nullptr;
    const char *typeTranslated = nullptr;
    if (pCallbackData->pMessage != nullptr) {
      std::string message;
      for (const auto &item : loggerCallbacks) {
        switch (item->getCallBackMode()) {
        case RAW_VULKAN_DEFS:
          item->messageRaw(messageSeverity, messageType, pCallbackData,
                           nullptr);
          break;
        case TRANSLATED_DEFS:
          if (typeTranslated == nullptr || severityTranslated == nullptr) {
            severityTranslated = translateSeverity(messageSeverity);
            typeTranslated = translateType(messageType);
            if (pCallbackData->pMessage != NULL) {
              message = std::string(pCallbackData->pMessage);
            }
          }
          item->translatedMessage(severityTranslated, typeTranslated, message);
          break;
        case BOTH_DEFS:
          if (typeTranslated == nullptr || severityTranslated == nullptr) {
            severityTranslated = translateSeverity(messageSeverity);
            typeTranslated = translateType(messageType);
            message = std::string(pCallbackData->pMessage);
          }
          item->messageRaw(messageSeverity, messageType, pCallbackData,
                           nullptr);
          item->translatedMessage(severityTranslated, typeTranslated, message);
          break;
        }
      }
      if (loggerCallbacks.empty()) {
        if (typeTranslated == nullptr || severityTranslated == nullptr) {
          severityTranslated = translateSeverity(messageSeverity);
          typeTranslated = translateType(messageType);
          message = std::string(pCallbackData->pMessage);
        }
        defaultCallback.translatedMessage(severityTranslated, typeTranslated,
                                          message);
      }
    }

    return VK_FALSE;
  }
  const char *
  translateSeverity(VkDebugUtilsMessageSeverityFlagBitsEXT severity) {
    const char *res;
    switch (severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      res = "VERBOSE";
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      res = "ERROR";
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      res = "WARNING";
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      res = "INFO";
      break;
    default:
      res = "UNDEFINED";
      break;
    }
    return res;
  }
  const char *translateType(VkDebugUtilsMessageTypeFlagsEXT type) {
    const char *res = "";
    switch (type) {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
      res = "GENERAL";
      break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
      res = "PERFORMANCE";
      break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
      res = "VALIDATION";
      break;
    default:
      res = "UNDEFINED";
      break;
    }
    return res;
  }

public:
  void destroy() override {
    instance.destroyDebugUtilsMessengerEXT(messenger, VkLibAlloc::acquireAllocCb().get(), loaderDynamic);
    destroyed = true;
  }

  virtual ~InstanceLogger() {}
};
