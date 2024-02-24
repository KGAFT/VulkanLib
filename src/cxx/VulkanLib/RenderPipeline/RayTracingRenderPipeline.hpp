//
// Created by kgaft on 24.02.24.
//

#ifndef VULKANRENDERENGINE_RAYTRACINGRENDERPIPELINE_HPP
#define VULKANRENDERENGINE_RAYTRACINGRENDERPIPELINE_HPP

#include "VulkanLib/Instance.hpp"
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "VulkanLib/Pipelines/RayTracingPipeline/RayTracingPipelineBuilder.hpp"
#include "VulkanLib/Pipelines/Shader.hpp"
#include "VulkanLib/Pipelines/RayTracingPipeline/RayTracingPipeline.hpp"
#include "VulkanLib/Pipelines/RayTracingPipeline/ShaderBindingTable.hpp"

namespace vkLibRt{
    class RayTracingRenderPipeline : public IDestroyableObject {
    public:
        RayTracingRenderPipeline(Instance& instance, std::shared_ptr<LogicalDevice> device, std::shared_ptr<RayTracingPipelineBuilder> builder, Shader* shader): instance(instance), device(device){
            rayTracingPipeline = std::make_shared<RayTracingPipeline>(instance, device,
                                                                               shader, builder);
            shaderBindingTable = std::make_shared<ShaderBindingTable>(device, instance, shader,
                                                                               rayTracingPipeline->getRayTracingPipelineKhr());
        }
    private:
        std::shared_ptr<RayTracingPipeline> rayTracingPipeline;
        std::shared_ptr<ShaderBindingTable> shaderBindingTable;
        Instance& instance;
        std::shared_ptr<LogicalDevice> device;
        vk::StridedDeviceAddressRegionKHR callRegion{};
    public:
        void beginRayTrace(vk::CommandBuffer cmd){
            cmd.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, rayTracingPipeline->getRayTracingPipelineKhr());
        }
        void rayTrace(vk::CommandBuffer cmd, uint32_t width, uint32_t height){
            cmd.traceRaysKHR(shaderBindingTable->getSbtRegions()[vk::ShaderStageFlagBits::eRaygenKHR], shaderBindingTable->getSbtRegions()[vk::ShaderStageFlagBits::eMissKHR],
                             shaderBindingTable->getSbtRegions()[vk::ShaderStageFlagBits::eAnyHitKHR], callRegion, width, height, 1,
                             instance.getDynamicLoader());
        }

        std::shared_ptr<RayTracingPipeline> getRayTracingPipeline()  {
            return rayTracingPipeline;
        }

    public:
        void destroy() override {
            shaderBindingTable->destroy();
            rayTracingPipeline->destroy();
            destroyed = true;
        }
    };
}



#endif //VULKANRENDERENGINE_RAYTRACINGRENDERPIPELINE_HPP
