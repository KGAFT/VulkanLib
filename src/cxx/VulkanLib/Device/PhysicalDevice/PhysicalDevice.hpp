//
// Created by kgaft on 11/4/23.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include "VulkanLib/Instance.hpp"
#include <vector>

class PhysicalDevice {
private:
    static inline std::vector<PhysicalDevice*> physicalDevices = std::vector<PhysicalDevice*>();
public:
    static const std::vector<PhysicalDevice*>& getDevices(Instance& instance) {
        if(physicalDevices.empty()){
            uint32_t physDevCount;
            instance.getInstance().enumeratePhysicalDevices(&physDevCount, nullptr);
            physicalDevices.resize(physDevCount);
            std::vector<vk::PhysicalDevice> tempDevices;
            tempDevices.resize(physDevCount);
            instance.getInstance().enumeratePhysicalDevices(&physDevCount,
                                                            tempDevices.data());
            for (int i = 0; i < tempDevices.size(); ++i){
                physicalDevices[i] = new PhysicalDevice(tempDevices[i]);
            }
        }
        return physicalDevices;
    }
private:
    PhysicalDevice(){

    }
    PhysicalDevice(vk::PhysicalDevice base) : base(base) {
        uint32_t propertyCount;
        base.enumerateDeviceExtensionProperties(nullptr, &propertyCount, nullptr);
        extensionProperties.resize(propertyCount);
        base.enumerateDeviceExtensionProperties(nullptr, &propertyCount, extensionProperties.data());
        base.getFeatures(&features);
        base.getProperties(&properties);
        uint32_t queueFamilyCount;
        base.getQueueFamilyProperties(&queueFamilyCount, nullptr);
        queueProperties.resize(queueFamilyCount);
        base.getQueueFamilyProperties(&queueFamilyCount, queueProperties.data());

    }
    void init(vk::PhysicalDevice base){
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


