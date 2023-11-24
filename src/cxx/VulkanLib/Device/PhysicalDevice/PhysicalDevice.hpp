//
// Created by kgaft on 11/4/23.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include "VulkanLib/Instance.hpp"
#include <vector>

class PhysicalDevice {
private:
public:
    static std::shared_ptr<std::vector<std::shared_ptr<PhysicalDevice>>> getDevices(Instance &instance) {
        std::shared_ptr<std::vector<std::shared_ptr<PhysicalDevice>>> physicalDevices(
                new std::vector<std::shared_ptr<PhysicalDevice>>);
        uint32_t physDevCount;
        vk::Result res = instance.getInstance().enumeratePhysicalDevices(&physDevCount, nullptr);
        physicalDevices->resize(physDevCount);
        std::vector<vk::PhysicalDevice> tempDevices;
        tempDevices.resize(physDevCount);
        res = instance.getInstance().enumeratePhysicalDevices(&physDevCount,
                                                              tempDevices.data());
        for (int i = 0; i < tempDevices.size(); ++i) {
            (*physicalDevices)[i] = std::shared_ptr<PhysicalDevice>(new PhysicalDevice(tempDevices[i]));
        }
        return physicalDevices;

    }


private:
    PhysicalDevice() = default;

    explicit PhysicalDevice(vk::PhysicalDevice base) : base(base) {
        uint32_t propertyCount;
        vk::Result res;
        res = base.enumerateDeviceExtensionProperties(nullptr, &propertyCount, nullptr);
        extensionProperties.resize(propertyCount);
        res = base.enumerateDeviceExtensionProperties(nullptr, &propertyCount, extensionProperties.data());
        base.getFeatures(&features);
        base.getProperties(&properties);
        uint32_t queueFamilyCount;
        base.getQueueFamilyProperties(&queueFamilyCount, nullptr);
        queueProperties.resize(queueFamilyCount);
        base.getQueueFamilyProperties(&queueFamilyCount, queueProperties.data());

    }

    void init(vk::PhysicalDevice base) {
        vk::Result res;
        PhysicalDevice::base = base;
        uint32_t propertyCount;
        res = base.enumerateDeviceExtensionProperties(nullptr, &propertyCount, nullptr);
        extensionProperties.resize(propertyCount);
        res = base.enumerateDeviceExtensionProperties(nullptr, &propertyCount, extensionProperties.data());
        base.getFeatures(&features);
        base.getProperties(&properties);
    }

public:
    vk::PhysicalDevice base;
    vk::PhysicalDeviceProperties properties;
    vk::PhysicalDeviceFeatures features;
    std::vector<vk::ExtensionProperties> extensionProperties;
    std::vector<vk::QueueFamilyProperties> queueProperties;
public:
    vk::PhysicalDevice &getBase() {
        return base;
    }

    const vk::PhysicalDeviceProperties &getProperties() const {
        return properties;
    }

    const vk::PhysicalDeviceFeatures &getFeatures() const {
        return features;
    }

    const std::vector<vk::ExtensionProperties> &getExtensionProperties() const {
        return extensionProperties;
    }
};


