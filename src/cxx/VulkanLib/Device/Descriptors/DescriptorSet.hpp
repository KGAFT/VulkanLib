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

class DescriptorSet {
    friend class DescriptorPool;

private:
    std::vector<vk::DescriptorSet> descriptorSet;
    std::vector<DescriptorBufferInfo> buffersInfo;
    std::vector<DescriptorImageInfo> imagesInfo;
    std::vector<vk::WriteDescriptorSet> writes;
    std::shared_ptr<LogicalDevice> device;
public:
    /**
     * Warning: after this operation all functions for adding descriptor elements info will be unAvailable
     */
    void updateDescriptors() {
        if (writes.empty()) {
            populateWritesInfo();
        }
        for (auto &item: descriptorSet) {
            for (auto &cItem: writes) {
                cItem.dstSet = item;
            }
            device->getDevice().updateDescriptorSets(writes.size(), writes.data(), 0, nullptr);
        }
    }

    void bindDescriptor(vk::PipelineBindPoint bindPoint, uint32_t currentInstance, vk::CommandBuffer cmd,
                        vk::PipelineLayout pipelineLayout) {
        cmd.bindDescriptorSets(bindPoint, pipelineLayout, 0, 1, &descriptorSet[currentInstance], 0, nullptr);
    }

    void addBufferInfo(DescriptorBufferInfo bufferInfo){
        buffersInfo.push_back(bufferInfo);
    }

    void addImageInfo(DescriptorImageInfo imageInfo){
        imagesInfo.push_back(imageInfo);
    }
private:
    void populateWritesInfo() {
        writes.resize(imagesInfo.size() + buffersInfo.size());
        uint32_t i = 0;
        for (auto &item: buffersInfo) {
            writes[i].sType = vk::StructureType::eWriteDescriptorSet;
            writes[i].dstSet = descriptorSet[0];
            writes[i].dstBinding = item.binding;
            writes[i].dstArrayElement = 0;
            writes[i].descriptorType = item.descriptorType;
            writes[i].descriptorCount = 1;
            writes[i].pBufferInfo = &item.base;
            i++;
        }
        for (auto &item: imagesInfo) {
            writes[i].sType = vk::StructureType::eWriteDescriptorSet;
            writes[i].dstSet = descriptorSet[0];
            writes[i].dstBinding = item.binding;
            writes[i].dstArrayElement = 0;
            writes[i].descriptorType = item.descriptorType;
            writes[i].descriptorCount = 1;
            writes[i].pImageInfo = &item.base;
            i++;
        }
    }
};


