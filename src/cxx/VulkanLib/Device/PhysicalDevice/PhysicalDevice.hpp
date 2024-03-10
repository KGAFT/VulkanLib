//
// Created by kgaft on 11/4/23.
//

#ifndef VULKANLIB_PHYSICALDEVICE_HPP
#define VULKANLIB_PHYSICALDEVICE_HPP

#include <vulkan/vulkan.hpp>
#include "VulkanLib/Instance.hpp"
#include <vector>
#include <memory>

class PhysicalDevice {
private:
public:
    static std::shared_ptr<std::vector<std::shared_ptr<PhysicalDevice>>> getDevices(Instance &instance);


private:
    PhysicalDevice() = default;

    PhysicalDevice(vk::PhysicalDevice base);

    void init(vk::PhysicalDevice base);

public:
    vk::PhysicalDevice base;
    vk::PhysicalDeviceProperties properties;
    vk::PhysicalDeviceFeatures features;
    std::vector<vk::ExtensionProperties> extensionProperties;
    std::vector<vk::QueueFamilyProperties> queueProperties;
    vk::PhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelinePropertiesKhr{};
public:
    vk::PhysicalDevice &getBase();

    const vk::PhysicalDeviceProperties &getProperties() const;

    const vk::PhysicalDeviceFeatures &getFeatures() const;

    const std::vector<vk::ExtensionProperties> &getExtensionProperties() const;

    const vk::PhysicalDeviceRayTracingPipelinePropertiesKHR &getRayTracingPipelinePropertiesKhr() const;
};


#endif