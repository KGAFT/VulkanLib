//
// Created by kgaft on 11/14/23.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include "VulkanLib/MemoryUtils/IDestroyableObject.hpp"
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"

class Shader : public IDestroyableObject{
public:
    Shader(std::vector<vk::PipelineShaderStageCreateInfo>& shaderModules, LogicalDevice& device) : createInfos(shaderModules), device(device){

    }
private:
    std::vector<vk::PipelineShaderStageCreateInfo> createInfos;
    LogicalDevice& device;
    bool rtSorted = false;
public:
    std::vector<vk::PipelineShaderStageCreateInfo> &getCreateInfos() {
        return createInfos;
    }
    void sortRayTracingShadersByGroups(){
        std::vector<vk::PipelineShaderStageCreateInfo> newStages;
        std::vector<vk::PipelineShaderStageCreateInfo*> rayGenGroup;
        std::vector<vk::PipelineShaderStageCreateInfo*> hitGroup;
        std::vector<vk::PipelineShaderStageCreateInfo*> missGroup;
        std::vector<vk::PipelineShaderStageCreateInfo*> intersectionGroup;
        for (auto &item: createInfos){
            switch(item.stage){
                case vk::ShaderStageFlagBits::eMissKHR:
                    missGroup.push_back(&item);
                    break;
                case vk::ShaderStageFlagBits::eRaygenKHR:
                    rayGenGroup.push_back(&item);
                    break;
                case vk::ShaderStageFlagBits::eClosestHitKHR:
                    hitGroup.push_back(&item);
                    break;
                case vk::ShaderStageFlagBits::eAnyHitKHR:
                    hitGroup.push_back(&item);
                    break;
                case vk::ShaderStageFlagBits::eIntersectionKHR:
                    intersectionGroup.push_back(&item);
                    break;
                default:
                    break;
            }
        }
        for (auto &item: rayGenGroup){
            newStages.push_back(*item);
        }
        for (auto &item: hitGroup){
            newStages.push_back(*item);
        }
        for (auto &item: missGroup){
            newStages.push_back(*item);
        }
        for (auto &item: intersectionGroup){
            newStages.push_back(*item);
        }
        createInfos = newStages;
        rtSorted = true;
    }

    bool isSortedForRayTracing() const {
        return rtSorted;
    }

public:
    void destroy() override {
        destroyed = true;
        for (auto &item: createInfos){
            device.getDevice().destroyShaderModule(item.module, VkLibAlloc::acquireAllocCb().get());
        }
    }

    virtual ~Shader() {

    }
};


