//
// Created by kgaft on 3/11/24.
//
#include "PhysicalDevice.hpp"

std::shared_ptr<std::vector<std::shared_ptr<PhysicalDevice>>> PhysicalDevice::getDevices(Instance &instance) {
    std::shared_ptr<std::vector<std::shared_ptr<PhysicalDevice>>> physicalDevices(
            new std::vector<std::shared_ptr<PhysicalDevice>>);
    uint32_t physDevCount;
    vk::Result res = instance.getInstance().enumeratePhysicalDevices(&physDevCount, nullptr);
    if(res!=vk::Result::eSuccess){
        throw std::runtime_error("Failed to find any devices");
    }
    physicalDevices->resize(physDevCount);
    std::vector<vk::PhysicalDevice> tempDevices;
    tempDevices.resize(physDevCount);
    res = instance.getInstance().enumeratePhysicalDevices(&physDevCount,
                                                          tempDevices.data());
    for (uint32_t i = 0; i < tempDevices.size(); ++i) {
        (*physicalDevices)[i] = std::shared_ptr<PhysicalDevice>(new PhysicalDevice(tempDevices[i]));
    }
    return physicalDevices;

}

PhysicalDevice::PhysicalDevice(vk::PhysicalDevice base) : base(base) {
    uint32_t propertyCount;
    vk::Result res;
    res = base.enumerateDeviceExtensionProperties(nullptr, &propertyCount, nullptr);
    if(res!=vk::Result::eSuccess){
        throw std::runtime_error("Failed to find any extensions");
    }
    extensionProperties.resize(propertyCount);
    res = base.enumerateDeviceExtensionProperties(nullptr, &propertyCount, extensionProperties.data());
    if(res!=vk::Result::eSuccess){
        throw std::runtime_error("Failed to find any extensions");
    }
    base.getFeatures(&features);
    base.getProperties(&properties);
    uint32_t queueFamilyCount;
    base.getQueueFamilyProperties(&queueFamilyCount, nullptr);
    queueProperties.resize(queueFamilyCount);
    base.getQueueFamilyProperties(&queueFamilyCount, queueProperties.data());
    vk::PhysicalDeviceProperties2 prop2{};
    prop2.pNext = &rayTracingPipelinePropertiesKhr;
    base.getProperties2(&prop2);

}

void PhysicalDevice::init(vk::PhysicalDevice base) {
    vk::Result res;
    PhysicalDevice::base = base;
    uint32_t propertyCount;
    res = base.enumerateDeviceExtensionProperties(nullptr, &propertyCount, nullptr);
    if(res!=vk::Result::eSuccess){
        std::cerr<<"Failed to gather some device info"<<std::endl;
    }
    extensionProperties.resize(propertyCount);
    res = base.enumerateDeviceExtensionProperties(nullptr, &propertyCount, extensionProperties.data());
    if(res!=vk::Result::eSuccess){
        std::cerr<<"Failed to gather some device info"<<std::endl;
    }
    base.getFeatures(&features);
    base.getProperties(&properties);
    vk::PhysicalDeviceProperties2 prop2{};
    prop2.pNext = &rayTracingPipelinePropertiesKhr;
    base.getProperties2(&prop2);

}

vk::PhysicalDevice &PhysicalDevice::getBase() {
    return base;
}

const vk::PhysicalDeviceProperties &PhysicalDevice::getProperties() const {
    return properties;
}

const vk::PhysicalDeviceFeatures &PhysicalDevice::getFeatures() const {
    return features;
}

const std::vector<vk::ExtensionProperties> &PhysicalDevice::getExtensionProperties() const {
    return extensionProperties;
}

const vk::PhysicalDeviceRayTracingPipelinePropertiesKHR &PhysicalDevice::getRayTracingPipelinePropertiesKhr() const {
    return rayTracingPipelinePropertiesKhr;
}
