//
// Created by kgaft on 2/17/24.
//

#ifndef VULKANRENDERENGINE_ASUTILS_HPP
#define VULKANRENDERENGINE_ASUTILS_HPP

#include "ASTypes.hpp"
#include "VulkanLib/Device/Buffer/IndexBuffer.hpp"
#include "VulkanLib/Device/Buffer/VertexBuffer.hpp"
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include <memory>

namespace vkLibRt {
    class ASUtils {
    public:
        static void cmdCreateBlas(std::shared_ptr<LogicalDevice> device,
                                  Instance &instance, vk::CommandBuffer cmdBuf,
                                  std::vector<uint32_t> indices,
                                  std::vector<BuildAccelerationStructure> &buildAs,
                                  VkDeviceAddress scratchAddress,
                                  vk::QueryPool queryPool) {
            if (queryPool) // For querying the compaction size
                device->getDevice().resetQueryPool(queryPool, 0,
                                                   static_cast<uint32_t>(indices.size()));
            uint32_t queryCnt{0};

            for (const auto &idx: indices) {
                // Actual allocation of buffer and acceleration structure.
                vk::AccelerationStructureCreateInfoKHR createInfo{};
                createInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
                createInfo.size =
                        buildAs[idx].sizeInfo.accelerationStructureSize; // Will be used to
                // allocate memory.
                buildAs[idx].as =
                        createAcceleration(device, createInfo, instance.getDynamicLoader());

                // BuildInfo #2 part
                buildAs[idx].buildInfo.dstAccelerationStructure =
                        buildAs[idx].as.accel; // Setting where the build lands
                buildAs[idx].buildInfo.scratchData.deviceAddress =
                        scratchAddress; // All build are using the same scratch buffer

                // Building the bottom-level-acceleration-structure
                cmdBuf.buildAccelerationStructuresKHR(1, &buildAs[idx].buildInfo,
                                                      &buildAs[idx].rangeInfo,
                                                      instance.getDynamicLoader());

                // Since the scratch buffer is reused across builds, we need a barrier to
                // ensure one build is finished before starting the next one.
                VkMemoryBarrier barrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
                barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
                barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
                vkCmdPipelineBarrier(
                        cmdBuf, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                        VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1,
                        &barrier, 0, nullptr, 0, nullptr);

                if (queryPool) {
                    // Add a query to find the 'real' amount of memory needed, use for
                    // compaction
                    cmdBuf.writeAccelerationStructuresPropertiesKHR(
                            1, &buildAs[idx].buildInfo.dstAccelerationStructure,
                            vk::QueryType::eAccelerationStructureCompactedSizeKHR, queryPool,
                            queryCnt++, instance.getDynamicLoader());
                }
            }
        }

        static vk::DeviceAddress
        getASAddress(std::shared_ptr<LogicalDevice> device,
                     vk::AccelerationStructureKHR accel,
                     vk::DispatchLoaderDynamic &loaderDynamic) {
            vk::AccelerationStructureDeviceAddressInfoKHR addressInfo{};
            addressInfo.accelerationStructure = accel;
            return device->getDevice().getAccelerationStructureAddressKHR(
                    addressInfo, loaderDynamic);
        }


        // Scratch buffer must be created without info
        static void cmdCreateTlas(std::shared_ptr<LogicalDevice> device,
                                  Instance &instance, vk::CommandBuffer cmdBuf,
                                  Buffer *scratchBuffer, uint32_t countInstance,
                                  vk::DeviceAddress instBufferAddr,
                                  vk::BuildAccelerationStructureFlagsKHR flags,
                                  bool update, AccelKHR &tlas) {
            // Wraps a device pointer to the above uploaded instances.
            vk::AccelerationStructureGeometryInstancesDataKHR instancesVk{};
            instancesVk.data.deviceAddress = instBufferAddr;

            // Put the above into a VkAccelerationStructureGeometryKHR. We need to put
            // the instances struct in a union and label it as instance data.
            vk::AccelerationStructureGeometryKHR topASGeometry{};
            topASGeometry.geometryType = vk::GeometryTypeKHR::eInstances;
            topASGeometry.geometry.instances = instancesVk;

            // Find sizes
            vk::AccelerationStructureBuildGeometryInfoKHR buildInfo{};
            buildInfo.flags = flags;
            buildInfo.geometryCount = 1;
            buildInfo.pGeometries = &topASGeometry;
            buildInfo.mode = update ? vk::BuildAccelerationStructureModeKHR::eUpdate
                                    : vk::BuildAccelerationStructureModeKHR::eBuild;
            buildInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;

            vk::AccelerationStructureBuildSizesInfoKHR sizeInfo{
                    VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
            device->getDevice().getAccelerationStructureBuildSizesKHR(
                    vk::AccelerationStructureBuildTypeKHR::eDevice, &buildInfo,
                    &countInstance, &sizeInfo, instance.getDynamicLoader());

            // Create TLAS
            if (update == false) {

                vk::AccelerationStructureCreateInfoKHR createInfo{};
                createInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
                createInfo.size = sizeInfo.accelerationStructureSize;

                tlas =
                        createAcceleration(device, createInfo, instance.getDynamicLoader());
            }

            // Allocate the scratch memory
            scratchBuffer->initialize(sizeInfo.buildScratchSize,
                                      vk::BufferUsageFlagBits::eStorageBuffer |
                                      vk::BufferUsageFlagBits::eShaderDeviceAddress,
                                      vk::MemoryPropertyFlagBits::eHostVisible |
                                      vk::MemoryPropertyFlagBits::eHostCoherent);

            vk::DeviceAddress scratchAddress =
                    scratchBuffer->getAddress(instance.getDynamicLoader());

            // Update build information
            buildInfo.srcAccelerationStructure = update ? tlas.accel : vk::AccelerationStructureKHR{};
            buildInfo.dstAccelerationStructure = tlas.accel;
            buildInfo.scratchData.deviceAddress = scratchAddress;

            // Build Offsets info: n instances
            vk::AccelerationStructureBuildRangeInfoKHR buildOffsetInfo{countInstance, 0,
                                                                       0, 0};
            const vk::AccelerationStructureBuildRangeInfoKHR *pBuildOffsetInfo =
                    &buildOffsetInfo;

            cmdBuf.buildAccelerationStructuresKHR(1, &buildInfo, &pBuildOffsetInfo,
                                                  instance.getDynamicLoader());
        }

        static void objectToVkGeometryKHR(std::shared_ptr<VertexBuffer> vBuffer,
                                          std::shared_ptr<IndexBuffer> indexBuffer,
                                          std::shared_ptr<Buffer> transformBuffer,
                                          BlasInput *pOutObjectGeomtry,
                                          vk::DispatchLoaderDynamic &loaderDynamic, size_t vertexOffset,
                                          size_t indexOffset, size_t transformOffset, uint32_t maxIndex, uint32_t maxVertex) {
            vk::DeviceAddress vertexAddress = vBuffer->getBufferAddress(loaderDynamic);
            vk::DeviceAddress indexAddress =
                    indexBuffer->getBufferAddress(loaderDynamic);

            uint32_t maxPrimitiveCount = maxIndex / 3;
            pOutObjectGeomtry->asGeometry.push_back({});
            pOutObjectGeomtry->asBuildOffsetInfo.push_back({});
            pOutObjectGeomtry->asGeometry[pOutObjectGeomtry->asGeometry.size() - 1]
                    .geometry.triangles.vertexFormat =
                    vBuffer->getFormat(); // vec3 vertex position data.
            pOutObjectGeomtry->asGeometry[pOutObjectGeomtry->asGeometry.size() - 1]
                    .geometry.triangles.vertexData.deviceAddress = vertexAddress + vertexOffset;
            pOutObjectGeomtry->asGeometry[pOutObjectGeomtry->asGeometry.size() - 1]
                    .geometry.triangles.vertexStride = vBuffer->getStepSize();
            // Describe index data (32-bit unsigned int)
            pOutObjectGeomtry->asGeometry[pOutObjectGeomtry->asGeometry.size() - 1]
                    .geometry.triangles.indexType = indexBuffer->getIndexType();
            pOutObjectGeomtry->asGeometry[pOutObjectGeomtry->asGeometry.size() - 1]
                    .geometry.triangles.indexData.deviceAddress = indexAddress + indexOffset;
            pOutObjectGeomtry->asGeometry[pOutObjectGeomtry->asGeometry.size() - 1]
                    .geometry.triangles.transformData =
                    transformBuffer->getAddress(loaderDynamic) + transformOffset;

            // Indicate identity transform by setting transformData to null device
            // pointer.
            // triangles.transformData = {};
            pOutObjectGeomtry->asGeometry[pOutObjectGeomtry->asGeometry.size() - 1]
                    .geometry.triangles.maxVertex = maxVertex;

            // Identify the above data as containing opaque triangles.
            pOutObjectGeomtry->asGeometry[pOutObjectGeomtry->asGeometry.size() - 1]
                    .geometryType = vk::GeometryTypeKHR::eTriangles;
            pOutObjectGeomtry->asGeometry[pOutObjectGeomtry->asGeometry.size() - 1]
                    .flags = vk::GeometryFlagBitsKHR::eOpaque;

            pOutObjectGeomtry
                    ->asBuildOffsetInfo[pOutObjectGeomtry->asBuildOffsetInfo.size() - 1]
                    .firstVertex = 0;
            pOutObjectGeomtry
                    ->asBuildOffsetInfo[pOutObjectGeomtry->asBuildOffsetInfo.size() - 1]
                    .primitiveCount = maxPrimitiveCount;
            pOutObjectGeomtry
                    ->asBuildOffsetInfo[pOutObjectGeomtry->asBuildOffsetInfo.size() - 1]
                    .primitiveOffset = 0;
            pOutObjectGeomtry
                    ->asBuildOffsetInfo[pOutObjectGeomtry->asBuildOffsetInfo.size() - 1]
                    .transformOffset = 0;
        }

        static void cmdCompactBlas(std::shared_ptr<LogicalDevice> device,
                                   vk::CommandBuffer cmdBuf,
                                   std::vector<uint32_t> indices,
                                   std::vector<BuildAccelerationStructure> &buildAs,
                                   VkQueryPool queryPool,
                                   vk::DispatchLoaderDynamic &loader) {
            uint32_t queryCtn{0};

            // Get the compacted size result back
            std::vector<VkDeviceSize> compactSizes(
                    static_cast<uint32_t>(indices.size()));

            vkGetQueryPoolResults(
                    device->getDevice(), queryPool, 0, (uint32_t) compactSizes.size(),
                    compactSizes.size() * sizeof(VkDeviceSize), compactSizes.data(),
                    sizeof(VkDeviceSize), VK_QUERY_RESULT_WAIT_BIT);

            for (auto idx: indices) {
                buildAs[idx].cleanupAS = buildAs[idx].as; // previous AS to destroy
                buildAs[idx].sizeInfo.accelerationStructureSize =
                        compactSizes[queryCtn++]; // new reduced size

                // Creating a compact version of the AS
                vk::AccelerationStructureCreateInfoKHR asCreateInfo{};
                asCreateInfo.size = buildAs[idx].sizeInfo.accelerationStructureSize;
                asCreateInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;

                buildAs[idx].as = createAcceleration(device, asCreateInfo, loader);

                // Copy the original BLAS to a compact version
                vk::CopyAccelerationStructureInfoKHR copyInfo{};
                copyInfo.src = buildAs[idx].buildInfo.dstAccelerationStructure;
                copyInfo.dst = buildAs[idx].as.accel;
                copyInfo.mode = vk::CopyAccelerationStructureModeKHR::eCompact;
                cmdBuf.copyAccelerationStructureKHR(copyInfo, loader);
            }
        }

        static AccelKHR
        createAcceleration(std::shared_ptr<LogicalDevice> device,
                           vk::AccelerationStructureCreateInfoKHR &accel_,
                           vk::DispatchLoaderDynamic &loader) {
            AccelKHR resultAccel{};
            // Allocating the buffer to hold the acceleration structure
            resultAccel.buffer = std::make_shared<Buffer>(
                    device, accel_.size,
                    vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR |
                    vk::BufferUsageFlagBits::eShaderDeviceAddress,
                    vk::MemoryPropertyFlagBits::eHostVisible |
                    vk::MemoryPropertyFlagBits::eHostCoherent);
            // Setting the buffer
            accel_.buffer = resultAccel.buffer->getBuffer();
            // Create the acceleration structure
            assert(device->getDevice().createAccelerationStructureKHR(
                    &accel_, nullptr, &resultAccel.accel, loader) ==
                   vk::Result::eSuccess);
            return resultAccel;
        }
    };
} // namespace vkLibRt
#endif // VULKANRENDERENGINE_ASUTILS_HPP
