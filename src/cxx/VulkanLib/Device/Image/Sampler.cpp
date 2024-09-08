//
// Created by kgaft on 3/11/24.
//
#include "Sampler.hpp"

Sampler::Sampler(std::shared_ptr<LogicalDevice> device) : device(device) {
    vk::SamplerCreateInfo *samplerInfo = createInfos.getObjectInstance();
    samplerInfo->sType = vk::StructureType::eSamplerCreateInfo;
    samplerInfo->magFilter = vk::Filter::eLinear;
    samplerInfo->minFilter = vk::Filter::eLinear;
    samplerInfo->addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo->addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo->addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo->anisotropyEnable = VK_FALSE;
    samplerInfo->maxAnisotropy = device->getBaseDevice()->properties.limits.maxSamplerAnisotropy;
    samplerInfo->borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo->unnormalizedCoordinates = VK_FALSE;
    samplerInfo->compareEnable = VK_FALSE;
    samplerInfo->compareOp = vk::CompareOp::eAlways;
    samplerInfo->mipmapMode = vk::SamplerMipmapMode::eLinear;

    vk::Result res = device->getDevice().createSampler(samplerInfo, VkLibAlloc::acquireAllocCb().get(), &sampler);
    if(res!=vk::Result::eSuccess){
        throw std::runtime_error("Failed to create sampler");
    }
    createInfos.releaseObjectInstance(samplerInfo);

}

Sampler::Sampler(std::shared_ptr<LogicalDevice> device, vk::SamplerCreateInfo &createInfo) : device(device) {


    sampler = device->getDevice().createSampler(createInfo);

}

void Sampler::destroy() {
    destroyed = true;
    device->getDevice().destroySampler(sampler);
}

vk::Sampler &Sampler::getSampler() {
    return sampler;
}
