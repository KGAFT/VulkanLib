package com.kgaft.VulkanLib.Pipelines.RayTracingPipeline.AccelerationStructures.ASTypes;

import com.kgaft.VulkanLib.Utils.LwjglObject;
import org.lwjgl.vulkan.VkAccelerationStructureBuildRangeInfoKHR;
import org.lwjgl.vulkan.VkAccelerationStructureGeometryKHR;

import java.lang.instrument.IllegalClassFormatException;
import java.util.ArrayList;
import java.util.List;

public class BlasInput {
    public List<LwjglObject<VkAccelerationStructureGeometryKHR>> asGeometry = new ArrayList<>();
    public List<LwjglObject<VkAccelerationStructureBuildRangeInfoKHR>> asBuildOffsetInfo = new ArrayList<>();
    public int buildAsFlags = 0;

    public LwjglObject<VkAccelerationStructureGeometryKHR.Buffer> getAsGeometryBuffer() throws IllegalClassFormatException {
        LwjglObject<VkAccelerationStructureGeometryKHR.Buffer> res = new LwjglObject<>(VkAccelerationStructureGeometryKHR.class, VkAccelerationStructureGeometryKHR.Buffer.class, asGeometry.size());
        asGeometry.forEach(element -> {
            res.get().put(element.get());
        });
        res.get().rewind();
        return res;
    }

    public LwjglObject<VkAccelerationStructureBuildRangeInfoKHR.Buffer> getAsBuildOffsetInfoBuffer() throws IllegalClassFormatException {
        LwjglObject<VkAccelerationStructureBuildRangeInfoKHR.Buffer> res =
        new LwjglObject<>(VkAccelerationStructureBuildRangeInfoKHR.class, VkAccelerationStructureBuildRangeInfoKHR.Buffer.class, asBuildOffsetInfo.size());
        asBuildOffsetInfo.forEach(element->{
            res.get().put(element.get());
        });
        res.get().rewind();
        return res;
    }
}
