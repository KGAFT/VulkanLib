//
// Created by kgaft on 11/12/23.
//
#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "VulkanLib/Pipelines/GraphicsPipeline/Configuration/GraphicsPipelineBuilder.hpp"

class PipelineConfigurer : public IDestroyableObject {
    friend class GraphicsPipeline;

public:
    PipelineConfigurer(LogicalDevice &device, PipelineBuilder *builder) : device(device) {
        loadDescriptorSetLayout(builder);
        loadPipelineLayout(builder);
        prepareBinding(builder->vertexInputs);
        prepareInputAttribs(builder->vertexInputs);
    }

private:
    vk::PipelineLayout pipelineLayout;
    vk::DescriptorSetLayout descriptorSetLayout{std::nullptr_t()};
    LogicalDevice &device;
    vk::VertexInputBindingDescription inputBindDesc{};
    std::vector<vk::VertexInputAttributeDescription> inputAttribDescs;
private:
    void loadDescriptorSetLayout(PipelineBuilder *endConfiguration) {
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        bindings.resize(endConfiguration->uniformBufferInfo.size() + endConfiguration->samplersInfo.size() +
                        endConfiguration->storageImagesInfos.size() +
                        endConfiguration->accelerationStructuresInfos.size() +
                        endConfiguration->storageBufferInfos.size());
        size_t counter = 0;
        for (auto &element: endConfiguration->uniformBufferInfo) {
            uboToBind(element, bindings[counter]);
            counter++;
        }
        for (auto &element: endConfiguration->samplersInfo) {
            samplerToBind(element, bindings[counter]);
            counter++;
        }
        for (auto &item: endConfiguration->accelerationStructuresInfos) {
            asToBind(item, bindings[counter]);
            counter++;
        }
        for (auto &item: endConfiguration->storageImagesInfos) {
            storageImageToBind(item, bindings[counter]);
            counter++;
        }
        for(auto& item : endConfiguration->storageBufferInfos){
            sboToBind(item, bindings[counter]);
            counter++;
        }
        if (!bindings.empty()) {
            vk::DescriptorSetLayoutCreateInfo layoutInfo;
            layoutInfo.bindingCount = bindings.size();
            layoutInfo.pBindings = bindings.data();

            descriptorSetLayout = device.getDevice().createDescriptorSetLayout(layoutInfo);


        }
    }

public:
    const vk::PipelineLayout &getPipelineLayout() const {
        return pipelineLayout;
    }

    const vk::DescriptorSetLayout &getDescriptorSetLayout() const {
        return descriptorSetLayout;
    }

    const vk::VertexInputBindingDescription &getInputBindDesc() const {
        return inputBindDesc;
    }

    const std::vector<vk::VertexInputAttributeDescription> &getInputAttribDescs() const {
        return inputAttribDescs;
    }

private:

    void loadPipelineLayout(PipelineBuilder *endConfiguration) {
        std::vector<vk::PushConstantRange> pushConstantRanges;
        pushConstantRanges.resize(endConfiguration->pushConstantInfos.size());
        size_t counter = 0;
        for (auto &element: endConfiguration->pushConstantInfos) {
            infoToRange(element, pushConstantRanges[counter]);
            counter++;
        }
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
        pipelineLayoutInfo.setLayoutCount = 0;
        if (descriptorSetLayout != VK_NULL_HANDLE) {
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        }

        pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();
        pipelineLayoutInfo.pushConstantRangeCount = pushConstantRanges.size();
        pipelineLayout = device.getDevice().createPipelineLayout(pipelineLayoutInfo);
    }

    void prepareBinding(std::vector<VertexInput> &inputs) {
        size_t size = 0;
        for (const auto &element: inputs) {
            size += element.typeSize * element.coordinatesAmount;
        }
        inputBindDesc.binding = 0;
        inputBindDesc.stride = size;
        inputBindDesc.inputRate = vk::VertexInputRate::eVertex;
    }

    void prepareInputAttribs(std::vector<VertexInput> &inputs) {
        size_t offsetCount = 0;
        size_t counter = 0;
        inputAttribDescs.resize(inputs.size());
        for (const auto &element: inputs) {

            inputAttribDescs[counter].binding = 0;
            inputAttribDescs[counter].location = element.location;
            inputAttribDescs[counter].offset = offsetCount;
            inputAttribDescs[counter].format = element.format;
            offsetCount += element.typeSize * element.coordinatesAmount;
            counter++;
        }
    }

    static void infoToRange(PushConstantInfo &info, vk::PushConstantRange &range) {
        range.size = info.size;
        range.offset = 0;
        range.stageFlags = info.shaderStages;
    }

    static void uboToBind(UniformBufferInfo &bufferInfo, vk::DescriptorSetLayoutBinding &result) {
        result.binding = bufferInfo.binding;
        result.descriptorType = vk::DescriptorType::eUniformBuffer;
        result.descriptorCount = bufferInfo.descriptorCount;
        result.stageFlags = bufferInfo.shaderStages;
    }

    static void sboToBind(StorageBufferInfo &bufferInfo, vk::DescriptorSetLayoutBinding &result) {
        result.binding = bufferInfo.binding;
        result.descriptorType = vk::DescriptorType::eStorageBuffer;
        result.descriptorCount = bufferInfo.descriptorCount;
        result.stageFlags = bufferInfo.stageFlags;
    }

    static void samplerToBind(SamplerInfo &samplerInfo, vk::DescriptorSetLayoutBinding &result) {
        result.binding = samplerInfo.binding;
        result.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        result.descriptorCount = samplerInfo.descriptorCount;
        result.stageFlags = samplerInfo.shaderStages;
    }

    static void asToBind(AccelerationStructureInfo &asInfo, vk::DescriptorSetLayoutBinding &result) {
        result.binding = asInfo.binding;
        result.descriptorType = vk::DescriptorType::eAccelerationStructureKHR;
        result.descriptorCount = asInfo.descriptorCount;
        result.stageFlags = asInfo.shaderStages;
    }

    static void storageImageToBind(StorageImageInfo &storageImageInfo, vk::DescriptorSetLayoutBinding &result) {
        result.binding = storageImageInfo.binding;
        result.descriptorType = vk::DescriptorType::eStorageImage;
        result.descriptorCount = storageImageInfo.descriptorCount;
        result.stageFlags = storageImageInfo.shaderStages;
    }

public:
    void destroy() override {
        destroyed = true;
        device.getDevice().destroyPipelineLayout(pipelineLayout);
        device.getDevice().destroyDescriptorSetLayout(descriptorSetLayout);
    }

    virtual ~PipelineConfigurer() = default;
};


