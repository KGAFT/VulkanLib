//
// Created by kgaft on 11/12/23.
//
#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "VulkanLib/MemoryUtils/SerialObject.hpp"
#include "GraphicsPipelineBuilder.hpp"

class GraphicsPipelineConfigurer : public IDestroyableObject{


public:
     GraphicsPipelineConfigurer(LogicalDevice &device, GraphicsPipelineBuilder* builder) : device(device) {
         loadDescriptorSetLayout(builder);
         loadPipelineLayout(builder);
         prepareBinding(builder->vertexInputs);
         prepareInputAttribs(builder->vertexInputs);
     }

private:
    vk::PipelineLayout pipelineLayout;
    vk::DescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    LogicalDevice &device;
    vk::VertexInputBindingDescription inputBindDesc{};
    std::vector<vk::VertexInputAttributeDescription> inputAttribDescs;
private:
    void loadDescriptorSetLayout(GraphicsPipelineBuilder *endConfiguration) {
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        bindings.resize(endConfiguration->uniformBufferInfo.size()+endConfiguration->samplersInfo.size());
        size_t counter = 0;
        for (auto &element: endConfiguration->uniformBufferInfo) {
            uboToBind(element, bindings[counter]);
            counter++;
        }
        for (auto &element: endConfiguration->samplersInfo) {
            samplerToBind(element, bindings[counter]);
            counter++;
        }
        if (!bindings.empty()) {
            vk::DescriptorSetLayoutCreateInfo layoutInfo;
            layoutInfo.bindingCount = bindings.size();
            layoutInfo.pBindings = bindings.data();

            descriptorSetLayout = device.getDevice().createDescriptorSetLayout(layoutInfo);


        }
    }

    void loadPipelineLayout(GraphicsPipelineBuilder *endConfiguration) {
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

    static void infoToRange(PushConstantInfo &info, vk::PushConstantRange& range) {
        range.size = info.size;
        range.offset = 0;
        range.stageFlags = info.shaderStages;
    }

    static void uboToBind(UniformBufferInfo &bufferInfo, vk::DescriptorSetLayoutBinding& result) {
        result.binding = bufferInfo.binding;
        result.descriptorType = vk::DescriptorType::eUniformBuffer;
        result.descriptorCount = 1;
        result.stageFlags = bufferInfo.shaderStages;
    }

    static void samplerToBind(SamplerInfo &samplerInfo, vk::DescriptorSetLayoutBinding& result) {
        result.binding = samplerInfo.binding;
        result.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        result.descriptorCount = 1;
        result.stageFlags = samplerInfo.shaderStages;
    }

public:
protected:
    void destroy() override {
        destroyed = true;
        device.getDevice().destroyPipelineLayout(pipelineLayout);
        device.getDevice().destroyDescriptorSetLayout(descriptorSetLayout);
    }
};


