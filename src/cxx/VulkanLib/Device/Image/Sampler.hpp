//
// Created by kgaft on 12/3/23.
//
#ifndef VULKANLIB_SAMPLER_HPP
#define VULKANLIB_SAMPLER_HPP

#include <VulkanLib/Device/LogicalDevice/LogicalDevice.hpp>
#include "VulkanLib/MemoryUtils/SeriesObject.hpp"

class Sampler : public IDestroyableObject {
private:
    static inline auto createInfos = SeriesObject<vk::SamplerCreateInfo>();
public:
    Sampler(std::shared_ptr<LogicalDevice> device);

    Sampler(std::shared_ptr<LogicalDevice> device, vk::SamplerCreateInfo &createInfo);

private:
    std::shared_ptr<LogicalDevice> device;
    vk::Sampler sampler;
public:
    vk::Sampler &getSampler();

    void destroy() override;
};

#endif