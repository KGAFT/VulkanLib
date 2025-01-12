package com.kgaft.VulkanLib.Pipelines.RayTracingPipeline.AccelerationStructures.ASTypes;

import com.kgaft.VulkanLib.Utils.LwjglObject;
import org.lwjgl.vulkan.VkAccelerationStructureBuildGeometryInfoKHR;
import org.lwjgl.vulkan.VkAccelerationStructureBuildRangeInfoKHR;
import org.lwjgl.vulkan.VkAccelerationStructureBuildSizesInfoKHR;

import java.lang.instrument.IllegalClassFormatException;

public class BuildAccelerationStructure {
    public LwjglObject<VkAccelerationStructureBuildGeometryInfoKHR> buildInfo = new LwjglObject<>(VkAccelerationStructureBuildGeometryInfoKHR.class);
    public LwjglObject<VkAccelerationStructureBuildSizesInfoKHR> sizeInfo = new LwjglObject<>(VkAccelerationStructureBuildSizesInfoKHR.class);
    public LwjglObject<VkAccelerationStructureBuildRangeInfoKHR> rangeInfo = new LwjglObject<>(VkAccelerationStructureBuildRangeInfoKHR.class);
    public AccelKHR as;
    public AccelKHR cleanupAs;

    public BuildAccelerationStructure() throws IllegalClassFormatException {
    }
}
