//
// Created by kgaft on 11/5/23.
//
#pragma once

#include <vulkan/vulkan.hpp>
#include "PhysicalDevice.hpp"
#include "VulkanLib/Device/DeviceBuilder.hpp"

struct QueueFamilyInfo {
    unsigned int index;
    vk::QueueFamilyProperties properties;
    bool supportPresentation;
};

struct DeviceSuitabilityResults {
    std::vector<QueueFamilyInfo> queuesInfo;
};

class DeviceSuitability {
public:
    static bool isDeviceSuitable(DeviceBuilder &builder, std::shared_ptr<PhysicalDevice> device,
                                 DeviceSuitabilityResults *pOutput) {
        for (const auto &item: builder.requestExtensions) {
            bool found = false;
            for (const auto &citem: device->extensionProperties) {
                if (!strcmp(citem.extensionName, item)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return false;
            }
        }
        bool graphicsFound = false;
        bool presentFound = false;
        bool computeFound = false;
        unsigned int queueCounter = 0;
        QueueFamilyInfo info{};
        for (const auto &item: device->queueProperties) {
            if (builder.requireGraphicsSupport) {
                if (item.queueFlags & vk::QueueFlagBits::eGraphics) {
                    graphicsFound = true;
                    info = {queueCounter, item, false};
                }
            }
            if (builder.requireComputeSupport) {
                if (item.queueFlags & vk::QueueFlagBits::eCompute) {
                    computeFound = true;
                    info = {queueCounter, item, false};
                }
            }
            if (builder.requirePresentSupport) {
                if (device->getBase().getSurfaceSupportKHR(queueCounter, builder.surfaceForPresentationCheck)) {
                    presentFound = true;
                    info = {queueCounter, item, true};
                }
            }

            if (pOutput != nullptr) {
                pOutput->queuesInfo.push_back(info);
            }
            if (graphicsFound == builder.requireGraphicsSupport && presentFound == builder.requirePresentSupport &&
                computeFound == builder.requireComputeSupport) {
                break;
            }
            queueCounter++;
        }
        if (!(graphicsFound == builder.requireGraphicsSupport && presentFound == builder.requirePresentSupport &&
              computeFound == builder.requireComputeSupport)) {
            return false;
        }
        return true;
    }
};
