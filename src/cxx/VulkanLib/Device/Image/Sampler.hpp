//
// Created by kgaft on 12/3/23.
//
#pragma once

#include <VulkanLib/Device/LogicalDevice/LogicalDevice.hpp>
#include "VulkanLib/MemoryUtils/SeriesObject.hpp"

class Sampler {
private:
    static inline SeriesObject<vk::SamplerCreateInfo> createInfos = SeriesObject<vk::SamplerCreateInfo>();
public:
    Sampler(std::shared_ptr<LogicalDevice> device) : device(device) {
        vk::SamplerCreateInfo *samplerInfo = createInfos.getObjectInstance();
        samplerInfo->sType = vk::StructureType::eSamplerCreateInfo;
        samplerInfo->magFilter = vk::Filter::eLinear;
        samplerInfo->minFilter = vk::Filter::eLinear;
        samplerInfo->addressModeU = vk::SamplerAddressMode::eRepeat;
        samplerInfo->addressModeV = vk::SamplerAddressMode::eRepeat;
        samplerInfo->addressModeW = vk::SamplerAddressMode::eRepeat;
        samplerInfo->anisotropyEnable = VK_TRUE;
        samplerInfo->maxAnisotropy = device->getBaseDevice()->properties.limits.maxSamplerAnisotropy;
        samplerInfo->borderColor = vk::BorderColor::eIntOpaqueBlack;
        samplerInfo->unnormalizedCoordinates = VK_FALSE;
        samplerInfo->compareEnable = VK_FALSE;
        samplerInfo->compareOp = vk::CompareOp::eAlways;
        samplerInfo->mipmapMode = vk::SamplerMipmapMode::eLinear;

        vk::Result res = device->getDevice().createSampler(samplerInfo, nullptr, &sampler);
        createInfos.releaseObjectInstance(samplerInfo);

    }

    Sampler(std::shared_ptr<LogicalDevice> device, vk::SamplerCreateInfo &createInfo) : device(device) {


        sampler = device->getDevice().createSampler(createInfo);

    }


private:
    std::shared_ptr<LogicalDevice> device;
    vk::Sampler sampler;
public:
    vk::Sampler &getSampler() {
        return sampler;
    }
};