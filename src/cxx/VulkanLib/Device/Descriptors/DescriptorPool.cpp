//
// Created by kgaft on 3/11/24.
//
#include "DescriptorPool.hpp"

std::shared_ptr<DescriptorPool> DescriptorPool::getInstance(std::shared_ptr<LogicalDevice> device, bool supportAccelerationStructures) {
    if(!initialized){
        instance = std::shared_ptr<DescriptorPool>(new DescriptorPool(device, supportAccelerationStructures));
        initialized = true;
    }
    return instance;
}

DescriptorPool::DescriptorPool(std::shared_ptr<LogicalDevice> logicalDevice, bool supportAccelerationStructures) : logicalDevice(logicalDevice) {
    std::vector<vk::DescriptorPoolSize> poolSizes = std::vector<vk::DescriptorPoolSize>();
    if(poolSizes.empty()){
        for(uint32_t i = 0; i<=10; i++){
            poolSizes.push_back({(vk::DescriptorType)i, 10000});
        }
        if(supportAccelerationStructures) {
            poolSizes.push_back({vk::DescriptorType::eAccelerationStructureKHR, 10000});

        }
    }
    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.maxSets = 1500;
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.poolSizeCount =(uint32_t) poolSizes.size();
    descriptorPool = logicalDevice->getDevice().createDescriptorPool(poolInfo, VkLibAlloc::acquireAllocCb().get());
}

std::shared_ptr<DescriptorSet>
DescriptorPool::allocateDescriptorSet(uint32_t instanceCount, vk::DescriptorSetLayout layout) {
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

void DescriptorPool::destroy() {
    destroyed = true;
    logicalDevice->getDevice().destroyDescriptorPool(descriptorPool, VkLibAlloc::acquireAllocCb().get());
}


