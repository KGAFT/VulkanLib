//
// Created by kgaft on 2/17/24.
//

#ifndef VULKANRENDERENGINE_ASTYPES_HPP
#define VULKANRENDERENGINE_ASTYPES_HPP

#include <vulkan/vulkan_core.h>
#include "VulkanLib/Device/Buffer/Buffer.hpp"

namespace vkLibRt {
    struct AccelKHR {
        vk::AccelerationStructureKHR accel = VK_NULL_HANDLE;
        std::shared_ptr<Buffer> buffer;
    };

    struct BlasInput {
        // Data used to build acceleration structure geometry
        std::vector<vk::AccelerationStructureGeometryKHR> asGeometry;
        std::vector<vk::AccelerationStructureBuildRangeInfoKHR> asBuildOffsetInfo;
        vk::BuildAccelerationStructureFlagsKHR flags{0};
    };

    struct BuildAccelerationStructure {
        vk::AccelerationStructureBuildGeometryInfoKHR buildInfo;
        vk::AccelerationStructureBuildSizesInfoKHR sizeInfo;
        const vk::AccelerationStructureBuildRangeInfoKHR *rangeInfo;
        AccelKHR as;  // result acceleration structure
        AccelKHR cleanupAS;
    };
}
#endif //VULKANRENDERENGINE_ASTYPES_HPP
