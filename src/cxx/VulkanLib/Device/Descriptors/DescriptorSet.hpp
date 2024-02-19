//
// Created by kgaft on 11/29/23.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"

struct DescriptorBufferInfo {
    vk::DescriptorBufferInfo base;
    uint32_t binding;
    vk::DescriptorType descriptorType;
};

struct DescriptorImageInfo {
    vk::DescriptorImageInfo base;
    uint32_t binding;
    vk::DescriptorType descriptorType;
};

struct DescriptorAccelerationStructureInfo{
    vk::WriteDescriptorSetAccelerationStructureKHR base;
    uint32_t binding;
    vk::DescriptorType descriptorType;
};

class DescriptorSet {
    friend class DescriptorPool;

private:
    std::vector<vk::DescriptorSet> descriptorSet;
    std::vector<DescriptorBufferInfo*> buffersInfo;
    std::vector<DescriptorImageInfo*> imagesInfo;
    std::vector<DescriptorAccelerationStructureInfo*> asInfo;

    std::vector<vk::WriteDescriptorSet> writes;
    std::shared_ptr<LogicalDevice> device;
    uint32_t imageInfoPerInstanceAmount = 0;
    uint32_t bufferInfoPerInstanceAmount = 0;

public:
    /**
     * Warning: after this operation all functions for adding descriptor elements info will be unAvailable
     */
    void updateDescriptors() {

        uint32_t counter = 0;
        for (auto &item: descriptorSet) {

            for (auto &cItem: writes) {
                cItem.dstSet = item;
            }
            if(imageInfoPerInstanceAmount==0 && bufferInfoPerInstanceAmount==0){
                device->getDevice().updateDescriptorSets(writes.size(), writes.data(), 0, nullptr);
            } else {
                device->getDevice().updateDescriptorSets(imageInfoPerInstanceAmount+bufferInfoPerInstanceAmount, &writes[counter], 0, nullptr);
            }
            counter+=imageInfoPerInstanceAmount+bufferInfoPerInstanceAmount;
        }
    }
    void clearObjectsInfos(){
        for (auto &item: imagesInfo){
            delete item;
        }
        imagesInfo.clear();
        for (const auto &item: buffersInfo){
            delete item;
        }
        buffersInfo.clear();
        asInfo.clear();
        writes.clear();
    }

    void bindDescriptor(vk::PipelineBindPoint bindPoint, uint32_t currentInstance, vk::CommandBuffer cmd,
                        vk::PipelineLayout pipelineLayout) {
        cmd.bindDescriptorSets(bindPoint, pipelineLayout, 0, 1, &descriptorSet[currentInstance], 0, nullptr);
    }

    void addBufferInfo(DescriptorBufferInfo& bufferInfo) {
        buffersInfo.push_back(new DescriptorBufferInfo);
        memcpy(buffersInfo[buffersInfo.size()-1], &bufferInfo, sizeof(DescriptorBufferInfo));
        writes.push_back({});
        writes[writes.size() - 1].sType = vk::StructureType::eWriteDescriptorSet;
        writes[writes.size() - 1].dstSet = descriptorSet[0];
        writes[writes.size() - 1].dstBinding = buffersInfo[buffersInfo.size()-1]->binding;
        writes[writes.size() - 1].dstArrayElement = 0;
        writes[writes.size() - 1].descriptorType = buffersInfo[buffersInfo.size()-1]->descriptorType;
        writes[writes.size() - 1].descriptorCount = 1;
        writes[writes.size() - 1].pBufferInfo = &buffersInfo[buffersInfo.size()-1]->base;
    }
    void addImageInfo(DescriptorImageInfo& imageInfo){
        imagesInfo.push_back(new DescriptorImageInfo);
        writes.push_back({});
        memcpy(imagesInfo[imagesInfo.size()-1], &imageInfo, sizeof(DescriptorImageInfo));

        writes[writes.size()-1].sType = vk::StructureType::eWriteDescriptorSet;
        writes[writes.size()-1].dstSet = descriptorSet[0];
        writes[writes.size()-1].dstBinding = imagesInfo[imagesInfo.size()-1]->binding;
        writes[writes.size()-1].dstArrayElement = 0;
        writes[writes.size()-1].descriptorType = imagesInfo[imagesInfo.size()-1]->descriptorType;
        writes[writes.size()-1].descriptorCount = 1;
        writes[writes.size()-1].pImageInfo = &imagesInfo[imagesInfo.size()-1]->base;
    }

    void addAccelerationStructureInfo(DescriptorAccelerationStructureInfo& acsInfo){
        asInfo.push_back(new DescriptorAccelerationStructureInfo);
        writes.push_back({});
        memcpy(&asInfo[asInfo.size()-1], &acsInfo, sizeof(DescriptorAccelerationStructureInfo));

        writes[writes.size()-1].sType = vk::StructureType::eWriteDescriptorSet;
        writes[writes.size()-1].dstSet = descriptorSet[0];
        writes[writes.size()-1].dstBinding = asInfo[asInfo.size()-1]->binding;
        writes[writes.size()-1].dstArrayElement = 0;
        writes[writes.size()-1].descriptorType = asInfo[asInfo.size()-1]->descriptorType;
        writes[writes.size()-1].descriptorCount = 1;
        writes[writes.size()-1].pNext = &asInfo[asInfo.size()-1]->base;
    }

    void setImageInfoPerInstanceAmount(uint32_t imageInfoPerInstanceAmount) {
        DescriptorSet::imageInfoPerInstanceAmount = imageInfoPerInstanceAmount;
    }

    void setBufferInfoPerInstanceAmount(uint32_t bufferInfoPerInstanceAmount) {
        DescriptorSet::bufferInfoPerInstanceAmount = bufferInfoPerInstanceAmount;
    }


};


