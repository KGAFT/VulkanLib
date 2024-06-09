//
// Created by kgaft on 6/9/24.
//

#ifndef COMPUTEPIPELINE_H
#define COMPUTEPIPELINE_H
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "VulkanLib/Pipelines/Shader.hpp"
#include "VulkanLib/Pipelines/PipelineConfiguration/PipelineBuilder.hpp"
#include "VulkanLib/Pipelines/PipelineConfiguration/PipelineConfigurer.hpp"


class ComputePipeline : public IDestroyableObject{
private:
    static inline SeriesObject<vk::ComputePipelineCreateInfo> createInfos = SeriesObject<vk::ComputePipelineCreateInfo>();
public:
    ComputePipeline(Instance& instance, std::shared_ptr<LogicalDevice> device, Shader *shader,
                    PipelineBuilder *pipelineBuilder) : device(device){
        configurer = std::make_shared<PipelineConfigurer>(*device.get(), pipelineBuilder);
        auto createInfo = createInfos.getObjectInstance();
        createInfo->sType = vk::StructureType::eComputePipelineCreateInfo;
        createInfo->layout = configurer->getPipelineLayout();
        createInfo->stage = shader->getCreateInfos()[0];
        auto result = device->getDevice().createComputePipeline(VK_NULL_HANDLE, *createInfo, nullptr, instance.getDynamicLoader());
        if(result.result!=vk::Result::eSuccess) {
            throw std::runtime_error("Failed to create compute pipeline");
        }
        computePipeline = result.value;
        createInfos.releaseObjectInstance(createInfo);
    }

private:
    std::shared_ptr<LogicalDevice> device;
    std::shared_ptr<PipelineConfigurer> configurer;
    vk::Pipeline computePipeline;

public:
    void bind(vk::CommandBuffer cmd) {
        cmd.bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline);
    }

    void dispatch(vk::CommandBuffer cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
        cmd.dispatch(groupCountX, groupCountY, groupCountZ);
    }

    void destroy() override {
        destroyed = true;
        device->getDevice().destroyPipeline(computePipeline);
        configurer->destroy();
    }

    virtual ~ComputePipeline() = default;
};


#endif //COMPUTEPIPELINE_H
