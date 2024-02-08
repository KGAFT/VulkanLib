//
// Created by kgaft on 11/28/23.
//
#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "VulkanLib/MemoryUtils/SeriesObject.hpp"
#include "DescriptorSet.hpp"

class DescriptorPool {
private:
    static inline std::shared_ptr<DescriptorPool> instance = std::shared_ptr<DescriptorPool>();
    static inline bool initialized = false;
public:
    static std::shared_ptr<DescriptorPool> getInstance(std::shared_ptr<LogicalDevice> device){
        if(!initialized){
            instance = std::shared_ptr<DescriptorPool>(new DescriptorPool(device));
            initialized = true;
        }
        return instance;
    }
private:
    DescriptorPool( std::shared_ptr<LogicalDevice> logicalDevice) : logicalDevice(logicalDevice) {
        std::vector<vk::DescriptorPoolSize> poolSizes = std::vector<vk::DescriptorPoolSize>();
        if(poolSizes.empty()){
            for(uint32_t i = 0; i<=10; i++){
                poolSizes.push_back({(vk::DescriptorType)i, 10000});
            }
          }
        vk::DescriptorPoolCreateInfo poolInfo{};
        poolInfo.maxSets = 1500;
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.poolSizeCount = poolSizes.size();
        descriptorPool = logicalDevice->getDevice().createDescriptorPool(poolInfo);
    }
    DescriptorPool(){

    }
private:
    vk::DescriptorPool descriptorPool;
    std::shared_ptr<LogicalDevice> logicalDevice;
    std::vector<vk::DescriptorSetLayout> layouts;
    SeriesObject<vk::DescriptorSetAllocateInfo> allocInfos;
public:
    std::shared_ptr<DescriptorSet> allocateDescriptorSet(uint32_t instanceCount, vk::DescriptorSetLayout layout){
        std::shared_ptr<DescriptorSet> res = std::make_shared<DescriptorSet>();
        layouts.clear();
        layouts.resize(instanceCount, layout);

        vk::DescriptorSetAllocateInfo* allocateInfo = allocInfos.getObjectInstance();
        allocateInfo->sType = vk::StructureType::eDescriptorSetAllocateInfo;
        allocateInfo->descriptorPool = descriptorPool;
        allocateInfo->pSetLayouts = layouts.data();
        allocateInfo->descriptorSetCount = instanceCount;

        res->descriptorSet.resize(instanceCount);
        res->device = logicalDevice;

        vk::Result allocRes = logicalDevice->getDevice().allocateDescriptorSets(allocateInfo, res->descriptorSet.data());
        if(allocRes!=vk::Result::eSuccess){
            throw std::runtime_error("Failed to allocate descriptor set");
        }
        return res;
    }
};


