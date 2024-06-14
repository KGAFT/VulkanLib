//
// Created by kgaft on 6/9/24.
//

#include "ComputePipeline.hpp"

ComputePipeline::ComputePipeline(Instance &instance, std::shared_ptr<LogicalDevice> device, Shader *shader,
    PipelineBuilder *pipelineBuilder): device(device) {
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

void ComputePipeline::bind(vk::CommandBuffer cmd) {
    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline);
}

void ComputePipeline::dispatch(vk::CommandBuffer cmd, uint32_t groupCountX, uint32_t groupCountY,
    uint32_t groupCountZ) {
    cmd.dispatch(groupCountX, groupCountY, groupCountZ);
}

void ComputePipeline::destroy() {
    destroyed = true;
    device->getDevice().destroyPipeline(computePipeline);
    configurer->destroy();
}
