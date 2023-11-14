//
// Created by kgaft on 11/14/23.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include "VulkanLib/MemoryUtils/IDestroyableObject.hpp"
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"

class Shader : IDestroyableObject{
public:
    Shader(std::vector<vk::PipelineShaderStageCreateInfo>& shaderModules, LogicalDevice& device) : createInfos(shaderModules), device(device){

    }
private:
    std::vector<vk::PipelineShaderStageCreateInfo> createInfos;
    LogicalDevice& device;
public:
    std::vector<vk::PipelineShaderStageCreateInfo> &getCreateInfos() {
        return createInfos;
    }

private:
    void destroy() override {
        destroyed = true;
        for (auto &item: createInfos){
            device.getDevice().destroyShaderModule(item.module);
        }
    }
};


