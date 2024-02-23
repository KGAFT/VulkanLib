//
// Created by UserName on 19.02.2024.
//

#ifndef VULKANRENDERENGINE_RAYTRACINGPIPELINE_HPP
#define VULKANRENDERENGINE_RAYTRACINGPIPELINE_HPP

#include <VulkanLib/Device/LogicalDevice/LogicalDevice.hpp>
#include "VulkanLib/Pipelines/Shader.hpp"
#include "VulkanLib/Pipelines/PipelineConfiguration/PipelineConfigurer.hpp"
#include "RayTracingPipelineBuilder.hpp"

namespace vkLibRt {
    class RayTracingPipeline {
    public:
        RayTracingPipeline(Instance& instance, std::shared_ptr<LogicalDevice> &device, Shader *shader,
                           std::shared_ptr<RayTracingPipelineBuilder> pBuilder) : device(device), configurer(*device,
                                                                                                             &pBuilder->pipelineBuilder) {
            initializePipeline(shader, pBuilder, instance.getDynamicLoader());
        }

    private:
        std::vector<vk::RayTracingShaderGroupCreateInfoKHR> shaderGroups;
        std::shared_ptr<LogicalDevice> device;
        PipelineConfigurer configurer;
        vk::Pipeline rayTracingPipelineKhr;
    private:
        void initializePipeline(Shader *shader, std::shared_ptr<RayTracingPipelineBuilder> pBuilder, vk::DispatchLoaderDynamic& dynamicLoader) {
            prepareShaderGroups(shader);
            vk::RayTracingPipelineCreateInfoKHR rayPipelineInfo{};
            rayPipelineInfo.stageCount = shader->getCreateInfos().size();  // Stages are shaders
            rayPipelineInfo.pStages = shader->getCreateInfos().data();
            rayPipelineInfo.groupCount = shaderGroups.size();
            rayPipelineInfo.pGroups = shaderGroups.data();

            rayPipelineInfo.maxPipelineRayRecursionDepth = pBuilder->maxRayRecursionDepth;  // Ray depth
            rayPipelineInfo.layout = configurer.getPipelineLayout();
            auto res = device->getDevice().createRayTracingPipelineKHR({}, {}, rayPipelineInfo, nullptr, dynamicLoader);
            if (res.result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create ray tracing pipeline");
            }
            rayTracingPipelineKhr = res.value;
        }

        void prepareShaderGroups(Shader* shader) {
            shaderGroups.clear();
            vk::RayTracingShaderGroupCreateInfoKHR group{};
            group.anyHitShader = VK_SHADER_UNUSED_KHR;
            group.closestHitShader = VK_SHADER_UNUSED_KHR;
            group.generalShader = VK_SHADER_UNUSED_KHR;
            group.intersectionShader = VK_SHADER_UNUSED_KHR;
            uint32_t i = 0;
            for (const auto &item: shader->getCreateInfos()) {
                switch (item.stage) {
                    case vk::ShaderStageFlagBits::eRaygenKHR:
                        group.type = vk::RayTracingShaderGroupTypeKHR::eGeneral;
                        group.generalShader = i;
                        group.anyHitShader = VK_SHADER_UNUSED_KHR;
                        group.closestHitShader = VK_SHADER_UNUSED_KHR;
                        group.intersectionShader = VK_SHADER_UNUSED_KHR;

                        break;
                    case vk::ShaderStageFlagBits::eMissKHR:
                        group.anyHitShader = VK_SHADER_UNUSED_KHR;
                        group.closestHitShader = VK_SHADER_UNUSED_KHR;
                        group.intersectionShader = VK_SHADER_UNUSED_KHR;
                        group.type = vk::RayTracingShaderGroupTypeKHR::eGeneral;
                        group.generalShader = i;
                        break;
                    case vk::ShaderStageFlagBits::eClosestHitKHR:
                        group.type = vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup;
                        group.generalShader = VK_SHADER_UNUSED_KHR;
                        group.closestHitShader = i;
                        break;
                    case vk::ShaderStageFlagBits::eAnyHitKHR:
                        group.type = vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup;
                        group.generalShader = VK_SHADER_UNUSED_KHR;
                        group.anyHitShader = i;
                        break;
                    case vk::ShaderStageFlagBits::eIntersectionKHR:
                        group.type = vk::RayTracingShaderGroupTypeKHR::eGeneral;
                        group.generalShader = VK_SHADER_UNUSED_KHR;
                        group.intersectionShader = i;
                        break;
                    default:
                        break;
                }
                i++;
                shaderGroups.push_back(group);
            }
        }
    public:
        vk::Pipeline getRayTracingPipelineKhr() {
            return rayTracingPipelineKhr;
        }
    };
}

#endif //VULKANRENDERENGINE_RAYTRACINGPIPELINE_H
