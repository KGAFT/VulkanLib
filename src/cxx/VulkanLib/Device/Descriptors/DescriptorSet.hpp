//
// Created by kgaft on 11/29/23.
//

#ifndef VULKANLIB_DESCRIPTORSET_HPP
#define VULKANLIB_DESCRIPTORSET_HPP

#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"

struct DescriptorBufferInfo {
    std::vector<vk::DescriptorBufferInfo> base;
    uint32_t binding;
    vk::DescriptorType descriptorType;
};

struct DescriptorImageInfo {
    std::vector<vk::DescriptorImageInfo> imageInfos;
    uint32_t binding;

    vk::DescriptorType descriptorType;
};

struct DescriptorAccelerationStructureInfo {
    vk::WriteDescriptorSetAccelerationStructureKHR base;
    uint32_t binding;
    vk::DescriptorType descriptorType;
};

class DescriptorSet {
    friend class DescriptorPool;

private:
    std::vector<vk::DescriptorSet> descriptorSet;
    std::vector<DescriptorBufferInfo *> buffersInfo;
    std::vector<DescriptorImageInfo *> imagesInfo;
    std::vector<DescriptorAccelerationStructureInfo *> asInfo;

    std::vector<vk::WriteDescriptorSet> writes;
    std::shared_ptr<LogicalDevice> device;
    uint32_t imageInfoPerInstanceAmount = 0;
    uint32_t bufferInfoPerInstanceAmount = 0;

public:
    void updateDescriptors();

    void clearObjectsInfos();

    void bindDescriptor(vk::PipelineBindPoint bindPoint, uint32_t currentInstance, vk::CommandBuffer cmd,
                        vk::PipelineLayout pipelineLayout);

    void addBufferInfo(DescriptorBufferInfo &bufferInfo);

    void addImageInfo(DescriptorImageInfo &imageInfo);

    void addAccelerationStructureInfo(DescriptorAccelerationStructureInfo &acsInfo);

    void setImageInfoPerInstanceAmount(uint32_t imageInfoPerInstanceAmount);

    void setBufferInfoPerInstanceAmount(uint32_t bufferInfoPerInstanceAmount);


};

#endif
