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
                    PipelineBuilder *pipelineBuilder);

private:
    std::shared_ptr<LogicalDevice> device;
    std::shared_ptr<PipelineConfigurer> configurer;
    vk::Pipeline computePipeline;

public:
    void bind(vk::CommandBuffer cmd);

    void dispatch(vk::CommandBuffer cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

    void destroy() override;

    std::shared_ptr<PipelineConfigurer> getConfigurer() {
        return configurer;
    }

    virtual ~ComputePipeline() = default;
};


#endif //COMPUTEPIPELINE_H
