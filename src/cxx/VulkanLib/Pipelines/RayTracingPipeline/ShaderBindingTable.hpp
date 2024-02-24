//
// Created by UserName on 21.02.2024.
//

#ifndef VULKANRENDERENGINE_SHADERBINDINGTABLE_HPP
#define VULKANRENDERENGINE_SHADERBINDINGTABLE_HPP


#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "VulkanLib/Pipelines/Shader.hpp"
#include "VulkanLib/Device/Buffer/Buffer.hpp"

namespace vkLibRt {

    class ShaderBindingTable : public IDestroyableObject{
    public:
        ShaderBindingTable(std::shared_ptr<LogicalDevice> device, Instance &instance, Shader *shader,
                           vk::Pipeline rayTracingPipeline) : sbtBuffer(device) {
            createShaderBindingTable(rayTracingPipeline, device, instance, shader);
        }

    private:
        Buffer sbtBuffer;
        std::map<vk::ShaderStageFlagBits, vk::StridedDeviceAddressRegionKHR> sbtRegions;
    public:
        std::map<vk::ShaderStageFlagBits, vk::StridedDeviceAddressRegionKHR> &getSbtRegions()  {
            return sbtRegions;
        }

    private:
        void
        createShaderBindingTable(vk::Pipeline rayTracingPipeline, std::shared_ptr<LogicalDevice> device,
                                 Instance &instance,
                                 Shader *shader) {
            size_t handleSize = device->getBaseDevice()->getRayTracingPipelinePropertiesKhr().shaderGroupHandleSize;
            size_t resHandleSize = MemoryUtils::alignUp(handleSize,
                                                        device->getBaseDevice()->getRayTracingPipelinePropertiesKhr().shaderGroupHandleAlignment);
            uint32_t handleCount = shader->getCreateInfos().size();

            std::map<vk::ShaderStageFlagBits, uint32_t> shaders;
            for (const auto &item: shader->getCreateInfos()) {
                vk::ShaderStageFlagBits stage = item.stage==vk::ShaderStageFlagBits::eClosestHitKHR?vk::ShaderStageFlagBits::eAnyHitKHR:item.stage;
                if (shaders.find(stage) == shaders.end()) {
                    shaders[stage] = 1;
                } else {
                    shaders[stage]++;
                }
            }
            size_t bufferSize = 0;

            for (const auto &item: shaders) {
                vk::StridedDeviceAddressRegionKHR regionKhr{};
                if (item.first == vk::ShaderStageFlagBits::eRaygenKHR) {
                    regionKhr.stride = MemoryUtils::alignUp(resHandleSize,
                                                            device->getBaseDevice()->getRayTracingPipelinePropertiesKhr().shaderGroupBaseAlignment);
                    regionKhr.size = regionKhr.stride*item.second;
                } else {
                    regionKhr.stride = resHandleSize;
                    regionKhr.size = MemoryUtils::alignUp(item.second * resHandleSize,
                                                          device->getBaseDevice()->getRayTracingPipelinePropertiesKhr().shaderGroupBaseAlignment);
                }
                bufferSize += regionKhr.size;
                sbtRegions[item.first] = regionKhr;
            }
            uint32_t dataSize = handleCount * handleSize;
            std::vector<uint8_t> handles(dataSize);
            auto result = device->getDevice().getRayTracingShaderGroupHandlesKHR(rayTracingPipeline, 0, handleCount,
                                                                               dataSize, handles.data(), instance.getDynamicLoader());

            assert(result == vk::Result::eSuccess);
            sbtBuffer.initialize(bufferSize,
                                 vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress |
                                 vk::BufferUsageFlagBits::eShaderBindingTableKHR,
                                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            vk::DeviceAddress bufferAddress = sbtBuffer.getAddress(instance.getDynamicLoader());
            size_t offsetCounter = 0;
            for (auto &item: sbtRegions) {
                item.second.deviceAddress = bufferAddress + offsetCounter;
                offsetCounter += item.second.size;
            }
            auto getHandle = [&](int i) { return handles.data() + i * handleSize; };
            void *sbtMapPoint;
            sbtBuffer.map(&sbtMapPoint, 0, vk::MemoryMapFlags());

            uint8_t *pData{nullptr};
            uint32_t handleIdx{0};

            pData = static_cast<uint8_t *>(sbtMapPoint);
            memcpy(pData, getHandle(handleIdx++), handleSize);
            pData = reinterpret_cast<uint8_t *>((uintptr_t) sbtMapPoint + (uintptr_t) sbtRegions[vk::ShaderStageFlagBits::eRaygenKHR].size);

            for (const auto &item: sbtRegions){
                memcpy(pData, getHandle(handleIdx++), handleSize);
                pData = reinterpret_cast<uint8_t *>((uintptr_t) pData + item.second.stride);
            }
            sbtBuffer.unMap();
        }

    public:
        void destroy() override {
            destroyed = true;
            sbtBuffer.destroy();
            sbtRegions.clear();
        }
    };
}

#endif //VULKANRENDERENGINE_SHADERBINDINGTABLE_HPP
