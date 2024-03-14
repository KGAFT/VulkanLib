//
// Created by UserName on 19.02.2024.
//

#ifndef VULKANRENDERENGINE_RAYTRACINGPIPELINEBUILDER_HPP
#define VULKANRENDERENGINE_RAYTRACINGPIPELINEBUILDER_HPP

#include <vulkan/vulkan.hpp>
#include "VulkanLib/Pipelines/PipelineConfiguration/PipelineBuilder.hpp"
namespace vkLibRt {

    class RayTracingPipelineBuilder {
        friend class RayTracingPipeline;

    private:
        PipelineBuilder pipelineBuilder;
        uint32_t maxRayRecursionDepth;
    public:
        void setMaxRayRecursionDepth(uint32_t maxRayRecursionDepth) {
            RayTracingPipelineBuilder::maxRayRecursionDepth = maxRayRecursionDepth;
        }

        void addStorageBufferInfo(StorageBufferInfo info){
            pipelineBuilder.addStorageBuffer(info);
        }

        void addVertexInput(VertexInput input) {
            pipelineBuilder.addVertexInput(input);
        }

        void addPushConstantInfo(PushConstantInfo info) {
            pipelineBuilder.addPushConstantInfo(info);
        }

        void addUniformBuffer(UniformBufferInfo info) {
            pipelineBuilder.addUniformBuffer(info);
        }

        void addSamplerInfo(SamplerInfo info) {
            pipelineBuilder.addSamplerInfo(info);
        }

        void addAccelerationStructureInfo(AccelerationStructureInfo info) {
            pipelineBuilder.addAccelerationStructureInfo(info);
        }

        void addStorageImageInfo(StorageImageInfo storageImageInfo) {
            pipelineBuilder.addStorageImageInfo(storageImageInfo);
        }
    };

}
#endif //VULKANRENDERENGINE_RAYTRACINGPIPELINEBUILDER_H
