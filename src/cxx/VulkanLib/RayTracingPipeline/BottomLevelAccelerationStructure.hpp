//
// Created by kgaft on 1/8/24.
//

#ifndef VULKANRENDERENGINE_BOTTOMLEVELACCELERATIONSTRUCTURE_HPP
#define VULKANRENDERENGINE_BOTTOMLEVELACCELERATIONSTRUCTURE_HPP

#include <vulkan/vulkan.hpp>
#include "VulkanLib/Device/Buffer/VertexBuffer.hpp"
#include "VulkanLib/Device/Buffer/IndexBuffer.hpp"

class BottomLevelAccelerationStructure {
private:
    static inline SeriesObject<vk::AccelerationStructureGeometryTrianglesDataKHR> trianglesInfos = SeriesObject<vk::AccelerationStructureGeometryTrianglesDataKHR>();
    static inline SeriesObject<vk::AccelerationStructureGeometryKHR> geometriesInfos = SeriesObject<vk::AccelerationStructureGeometryKHR>();
    static inline SeriesObject<vk::AccelerationStructureBuildRangeInfoKHR> buildRangeInfos = SeriesObject<vk::AccelerationStructureBuildRangeInfoKHR>();
    static inline SeriesObject<vk::AccelerationStructureBuildGeometryInfoKHR> buildGeometriesInfos = SeriesObject<vk::AccelerationStructureBuildGeometryInfoKHR>();
    static inline SeriesObject<vk::BufferCreateInfo> scratchBufferCreateInfos = SeriesObject<vk::BufferCreateInfo>();
    static inline SeriesObject<vk::AccelerationStructureCreateInfoKHR> accelerationCreateInfos = SeriesObject<vk::AccelerationStructureCreateInfoKHR>();
public:

    vk::AccelerationStructureKHR bottomLevelAccelerationStructure{};

    void build(std::shared_ptr<LogicalDevice> device, std::shared_ptr<VertexBuffer> vBuffer,
               std::shared_ptr<IndexBuffer> iBuffer, vk::DispatchLoaderDynamic &loader) {
        auto triangleInfo = trianglesInfos.getObjectInstance();
        triangleInfo->sType = vk::StructureType::eAccelerationStructureGeometryTrianglesDataKHR;
        triangleInfo->vertexFormat = vBuffer->getFormat();
        triangleInfo->vertexData.deviceAddress = vBuffer->getBufferAddress(loader);
        triangleInfo->vertexStride = vBuffer->getStepSize();
        triangleInfo->maxVertex = vBuffer->getVerticesAmount() - 1;
        triangleInfo->indexData.deviceAddress = iBuffer->getBufferAddress(loader);

        triangleInfo->indexType = iBuffer->getIndexType();
        triangleInfo->transformData = VK_NULL_HANDLE;
        auto geometryData = geometriesInfos.getObjectInstance();
        geometryData->sType = vk::StructureType::eAccelerationStructureGeometryKHR;

        geometryData->geometryType = vk::GeometryTypeKHR::eTriangles;
        geometryData->setGeometry(*triangleInfo);
        geometryData->flags = vk::GeometryFlagBitsKHR::eOpaque;

        auto buildRangeInfo = buildRangeInfos.getObjectInstance();
        buildRangeInfo->primitiveCount = vBuffer->getVerticesAmount();
        buildRangeInfo->primitiveOffset = 0;
        buildRangeInfo->firstVertex = 0;
        buildRangeInfo->transformOffset = 0;

        auto buildGeometryInfo = buildGeometriesInfos.getObjectInstance();
        buildGeometryInfo->sType = vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR;
        buildGeometryInfo->type = vk::AccelerationStructureTypeKHR::eBottomLevel;
        buildGeometryInfo->flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastBuild;
        buildGeometryInfo->mode = vk::BuildAccelerationStructureModeKHR::eBuild;
        buildGeometryInfo->srcAccelerationStructure = VK_NULL_HANDLE;
        buildGeometryInfo->geometryCount = 1;
        buildGeometryInfo->pGeometries = geometryData;
        buildGeometryInfo->ppGeometries = nullptr;
        buildGeometryInfo->dstAccelerationStructure = VK_NULL_HANDLE;

        vk::AccelerationStructureBuildSizesInfoKHR buildSize;
        device->getDevice().getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice,
                                                                  buildGeometryInfo, &buildRangeInfo->primitiveCount,
                                                                  &buildSize, loader);

        auto scratchBuffer = getScratchBuffer(device, buildSize.buildScratchSize);

        auto createInfo = accelerationCreateInfos.getObjectInstance();
        createInfo->sType = vk::StructureType::eAccelerationStructureCreateInfoKHR;
        createInfo->createFlags = vk::AccelerationStructureCreateFlagsKHR();
        createInfo->buffer = scratchBuffer->getBuffer();
        createInfo->offset = 0;
        createInfo->size = buildSize.buildScratchSize;
        createInfo->type = vk::AccelerationStructureTypeKHR::eBottomLevel;
        createInfo->deviceAddress = 0;
        device->getDevice().createAccelerationStructureKHR(createInfo, nullptr,
                                                           &bottomLevelAccelerationStructure, loader);
        buildGeometryInfo->dstAccelerationStructure = bottomLevelAccelerationStructure;
        buildGeometryInfo->scratchData.deviceAddress = scratchBuffer->getAddress(loader);
        vk::CommandBuffer cmd = device->getQueueByType(vk::QueueFlagBits::eGraphics)->beginSingleTimeCommands();
        cmd.buildAccelerationStructuresKHR(1, buildGeometryInfo,
                                           reinterpret_cast<const vk::AccelerationStructureBuildRangeInfoKHR *const *>(buildRangeInfo),
                                           loader);
        device->getQueueByType(vk::QueueFlagBits::eGraphics)->endSingleTimeCommands(cmd);
        trianglesInfos.releaseObjectInstance(triangleInfo);
        geometriesInfos.releaseObjectInstance(geometryData);
        buildRangeInfos.releaseObjectInstance(buildRangeInfo);
        buildGeometriesInfos.releaseObjectInstance(buildGeometryInfo);
        accelerationCreateInfos.releaseObjectInstance(createInfo);
        scratchBuffer->destroy();
    }

private:
    std::shared_ptr<Buffer> getScratchBuffer(std::shared_ptr<LogicalDevice> device, size_t size) {
        uint32_t queueIndices[] = {device->getQueueByType(vk::QueueFlagBits::eGraphics)->getIndex()};

        auto createInfo = scratchBufferCreateInfos.getObjectInstance();
        createInfo->sType = vk::StructureType::eBufferCreateInfo;
        createInfo->size = size;
        createInfo->usage = vk::BufferUsageFlagBits::eShaderDeviceAddressKHR | vk::BufferUsageFlagBits::eStorageBuffer |
                            vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR;
        createInfo->sharingMode = vk::SharingMode::eExclusive;
        createInfo->pQueueFamilyIndices = queueIndices;
        createInfo->queueFamilyIndexCount = 1;
        auto result = std::make_shared<Buffer>(device, createInfo, vk::MemoryPropertyFlags());
        scratchBufferCreateInfos.releaseObjectInstance(createInfo);
        return result;
    }
};


#endif //VULKANRENDERENGINE_BOTTOMLEVELACCELERATIONSTRUCTURE_HPP
