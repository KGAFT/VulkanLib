//
// Created by kgaft on 2/18/24.
//

#ifndef VULKANRENDERENGINE_TOPLEVELACCELERATIONSTRUCTURE_HPP
#define VULKANRENDERENGINE_TOPLEVELACCELERATIONSTRUCTURE_HPP

#include "ASTypes.hpp"
#include "ASUtils.hpp"
#include "BottomLevelAccelerationStructure.hpp"

namespace vkLibRt {
    class TopLevelAccelerationStructure : public IDestroyableObject {
    public:
        TopLevelAccelerationStructure(Instance &instance,
                                      std::shared_ptr<LogicalDevice> &device)
                : instance(instance), device(device) {}

    private:
        AccelKHR tlas;
        Instance &instance;
        std::shared_ptr<LogicalDevice> device;

    public:
        void createTlas(std::vector<AccelKHR> &blas) {
            std::vector<vk::AccelerationStructureInstanceKHR> instances;
            instances.resize(blas.size());
            VkTransformMatrixKHR transformMatrix = {1.0f, 0.0f, 0.0f, 0.0f,
                                                    0.0f, 1.0f, 0.0f, 0.0f,
                                                    0.0f, 0.0f, 1.0f, 0.0f};

            for (uint32_t i = 0; i < instances.size(); ++i) {
                MemoryUtils::memClear(&instances[i],
                                      sizeof(vk::AccelerationStructureInstanceKHR));

                instances[i].transform = transformMatrix;            // Position of the instance
                instances[i].instanceCustomIndex = 0; // gl_InstanceCustomIndexEXT
                instances[i].accelerationStructureReference = ASUtils::getASAddress(
                        device, blas[i].accel, instance.getDynamicLoader());
                instances[i].flags =
                        VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
                instances[i].mask = 0xFF; //  Only be hit if rayMask & instance.mask != 0
                instances[i].instanceShaderBindingTableRecordOffset = 0;
            }
            buildTlas(instance, device, instances);
        }

    public:
        void destroy() override {
            destroyed = true;
            device->getDevice().destroyAccelerationStructureKHR(
                    tlas.accel, nullptr, instance.getDynamicLoader());
            tlas.buffer->destroy();
        }

    public:
        vk::AccelerationStructureKHR &getAccelerationStructure() {
            return tlas.accel;
        }

    private:
        void
        buildTlas(Instance &instance, std::shared_ptr<LogicalDevice> device,
                  std::vector<vk::AccelerationStructureInstanceKHR> &instances,
                  vk::BuildAccelerationStructureFlagsKHR flags =
                  vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
                  bool update = false) {
            // Cannot call buildTlas twice except to update.
            assert(tlas.accel == VK_NULL_HANDLE || update);
            uint32_t countInstance = static_cast<uint32_t>(instances.size());

            // Command buffer to create the TLAS
            vk::CommandBuffer cmdBuf =
                    device->getQueueByType(vk::QueueFlagBits::eGraphics)
                            ->beginSingleTimeCommands();

            // Create a buffer holding the actual instance data (matrices++) for use by
            // the AS builder
            Buffer instancesBuffer(
                    device, instances.size() * sizeof(VkAccelerationStructureInstanceKHR),
                    vk::BufferUsageFlagBits::eShaderDeviceAddress |
                    vk::BufferUsageFlagBits::
                    eAccelerationStructureBuildInputReadOnlyKHR,
                    vk::MemoryPropertyFlagBits::eHostVisible |
                    vk::MemoryPropertyFlagBits::eHostCoherent); // Buffer of instances
            // containing the
            // matrices and BLAS ids
            void *instanceMap;
            instancesBuffer.map(&instanceMap, 0, vk::MemoryMapFlags());
            memcpy(instanceMap, instances.data(), instances.size() * sizeof(VkAccelerationStructureInstanceKHR));
            instancesBuffer.unMap();
            VkDeviceAddress instBufferAddr =
                    instancesBuffer.getAddress(instance.getDynamicLoader());

            // Make sure the copy of the instance buffer are copied before triggering
            // the acceleration structure build
            VkMemoryBarrier barrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
            vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                                 0, 1, &barrier, 0, nullptr, 0, nullptr);
            Buffer scratchBuffer(device);
            // Creating the TLAS
            ASUtils::cmdCreateTlas(device, instance, cmdBuf, &scratchBuffer,
                                   countInstance, instBufferAddr, flags, update, tlas);

            // Finalizing and destroying temporary data

            device->getQueueByType(vk::QueueFlagBits::eGraphics)
                    ->endSingleTimeCommands(cmdBuf);
            instancesBuffer.destroy();
            scratchBuffer.destroy();
        }
    };
} // namespace vkLibRt

#endif // VULKANRENDERENGINE_TOPLEVELACCELERATIONSTRUCTURE_HPP
