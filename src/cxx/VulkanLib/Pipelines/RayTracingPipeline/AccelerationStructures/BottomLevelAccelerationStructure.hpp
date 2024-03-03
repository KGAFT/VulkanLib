//
// Created by kgaft on 1/8/24.
//

#ifndef VULKANRENDERENGINE_BOTTOMLEVELACCELERATIONSTRUCTURE_HPP
#define VULKANRENDERENGINE_BOTTOMLEVELACCELERATIONSTRUCTURE_HPP

#include <memory>
#include <vulkan/vulkan.hpp>
#include <numeric>
#include "VulkanLib/Device/Buffer/VertexBuffer.hpp"
#include "VulkanLib/Device/Buffer/IndexBuffer.hpp"
#include "ASTypes.hpp"
#include "ASUtils.hpp"


namespace vkLibRt {

    class BottomLevelAccelerationStructure : public IDestroyableObject{
    public:
        BottomLevelAccelerationStructure(std::shared_ptr<LogicalDevice> device, Instance &instance) : device(device),
                                                                                                      instance(
                                                                                                              instance) {}

    private:
        std::shared_ptr<LogicalDevice> device;
        Instance &instance;
        SeriesObject<vk::BufferCreateInfo> bufferCreateInfos;
        std::vector<AccelKHR> accelerationStructures;
        std::vector<BlasInput> objectsInfos;
    public:
        void storeObject(std::shared_ptr<VertexBuffer> vBuffer, std::shared_ptr<IndexBuffer> iBuffer, std::shared_ptr<Buffer> transformBuffer,
                         size_t vertexOffset = 0, size_t indexOffset = 0, size_t transformOffset = 0, uint32_t maxIndex = 0, uint32_t maxVertex = 0) {
            objectsInfos.push_back({});
            uint32_t index = maxIndex==0?iBuffer->getIndexCount():maxIndex;
            uint32_t vertex = maxVertex==0?vBuffer->getVerticesAmount()-1:maxVertex;
            ASUtils::objectToVkGeometryKHR(vBuffer, iBuffer, transformBuffer, &objectsInfos[objectsInfos.size() - 1],
                                           instance.getDynamicLoader(), vertexOffset, indexOffset, transformOffset, index, vertex);
        }

        void confirmObjectsAndCreateBLASes() {
            buildBlas(instance, device, objectsInfos, vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);
            objectsInfos.clear();
        }

        std::vector<AccelKHR> &getAccelerationStructures() {
            return accelerationStructures;
        }

    public:
        void destroy() override {
            destroyed = true;
            for (const auto &item: accelerationStructures){
                device->getDevice().destroyAccelerationStructureKHR(item.accel, nullptr, instance.getDynamicLoader());
                item.buffer->destroy();
            }
            accelerationStructures.clear();

        }

    private:
        void buildBlas(Instance &instance, std::shared_ptr<LogicalDevice> device, std::vector<BlasInput> &input,
                       vk::BuildAccelerationStructureFlagsKHR flags) {
            auto nbBlas = static_cast<uint32_t>(input.size());
            VkDeviceSize asTotalSize{0};     // Memory size of all allocated BLAS
            uint32_t nbCompactions{0};   // Nb of BLAS requesting compaction
            VkDeviceSize maxScratchSize{0};  // Largest scratch size

            // Preparing the information for the acceleration build commands.
            std::vector<BuildAccelerationStructure> buildAs(nbBlas);
            for (uint32_t idx = 0; idx < nbBlas; idx++) {
                // Filling partially the VkAccelerationStructureBuildGeometryInfoKHR for querying the build sizes.
                // Other information will be filled in the createBlas (see #2)
                buildAs[idx].buildInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
                buildAs[idx].buildInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
                buildAs[idx].buildInfo.flags = input[idx].flags | flags;
                buildAs[idx].buildInfo.geometryCount = static_cast<uint32_t>(input[idx].asGeometry.size());
                buildAs[idx].buildInfo.pGeometries = input[idx].asGeometry.data();

                // Build range information
                buildAs[idx].rangeInfo = input[idx].asBuildOffsetInfo.data();

                // Finding sizes to create acceleration structures and scratch
                std::vector<uint32_t> maxPrimCount(input[idx].asBuildOffsetInfo.size());
                for (auto tt = 0; tt < input[idx].asBuildOffsetInfo.size(); tt++)
                    maxPrimCount[tt] = input[idx].asBuildOffsetInfo[tt].primitiveCount;  // Number of primitives/triangles
                device->getDevice().getAccelerationStructureBuildSizesKHR(
                        vk::AccelerationStructureBuildTypeKHR::eDevice,
                        &buildAs[idx].buildInfo, maxPrimCount.data(),
                        &buildAs[idx].sizeInfo, instance.getDynamicLoader());


                // Extra info
                asTotalSize += buildAs[idx].sizeInfo.accelerationStructureSize;
                maxScratchSize = std::max(maxScratchSize, buildAs[idx].sizeInfo.buildScratchSize);
                nbCompactions += (
                        (buildAs[idx].buildInfo.flags & vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction) ==
                        vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction);
            }
            Buffer scratchBuffer(device, maxScratchSize,
                                 vk::BufferUsageFlagBits::eShaderDeviceAddressKHR |
                                 vk::BufferUsageFlagBits::eStorageBuffer |
                                 vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR,
                                 vk::MemoryPropertyFlags());

            vk::DeviceAddress scratchAddress = scratchBuffer.getAddress(instance.getDynamicLoader());

            // Allocate a query pool for storing the needed size for every BLAS compaction.
            vk::QueryPool queryPool{VK_NULL_HANDLE};
            if (nbCompactions > 0)  // Is compaction requested?
            {
                assert(nbCompactions == nbBlas);  // Don't allow mix of on/off compaction
                vk::QueryPoolCreateInfo qpci{};
                qpci.queryCount = nbBlas;
                qpci.queryType = vk::QueryType::eAccelerationStructureCompactedSizeKHR;
                queryPool = device->getDevice().createQueryPool(qpci);
            }

            // Batching creation/compaction of BLAS to allow staying in restricted amount of memory
            std::vector<uint32_t> indices;  // Indices of the BLAS to create
            vk::DeviceSize batchSize{0};
            vk::DeviceSize batchLimit{256'000'000};  // 256 MB
            for (uint32_t idx = 0; idx < nbBlas; idx++) {
                indices.push_back(idx);
                batchSize += buildAs[idx].sizeInfo.accelerationStructureSize;
                // Over the limit or last BLAS element
                if (batchSize >= batchLimit || idx == nbBlas - 1) {
                    vk::CommandBuffer cmdBuf = device->getQueueByType(
                            vk::QueueFlagBits::eGraphics)->beginSingleTimeCommands();
                    ASUtils::cmdCreateBlas(device, instance, cmdBuf, indices, buildAs, scratchAddress, queryPool);
                    device->getQueueByType(
                            vk::QueueFlagBits::eGraphics)->endSingleTimeCommands(cmdBuf);

                    if (queryPool) {
                        vk::CommandBuffer cmdBuf = device->getQueueByType(
                                vk::QueueFlagBits::eGraphics)->beginSingleTimeCommands();
                        ASUtils::cmdCompactBlas(device, cmdBuf, indices, buildAs, queryPool,
                                                instance.getDynamicLoader());
                        device->getQueueByType(
                                vk::QueueFlagBits::eGraphics)->endSingleTimeCommands(cmdBuf);

                        // Destroy the non-compacted version
                        for (const auto &item: buildAs) {
                            device->getDevice().destroyAccelerationStructureKHR(item.cleanupAS.accel, nullptr,
                                                                                instance.getDynamicLoader());
                            item.cleanupAS.buffer->destroy();
                        }
                    }
                    // Reset

                    batchSize = 0;
                    indices.clear();
                }
            }

            // Logging reduction
            if (queryPool) {
                VkDeviceSize compactSize = std::accumulate(buildAs.begin(), buildAs.end(), 0ULL,
                                                           [](const auto &a, const auto &b) {
                                                               return a + b.sizeInfo.accelerationStructureSize;
                                                           });
                const float fractionSmaller = (asTotalSize == 0) ? 0 : (asTotalSize - compactSize) / float(asTotalSize);

            }

            // Keeping all the created acceleration structures
            for (auto &b: buildAs) {
                accelerationStructures.push_back(b.as);
            }

            scratchBuffer.destroy();
            device->getDevice().destroyQueryPool(queryPool);
        }


    };
}

#endif //VULKANRENDERENGINE_BOTTOMLEVELACCELERATIONSTRUCTURE_HPP
