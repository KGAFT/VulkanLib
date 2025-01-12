package com.kgaft.VulkanLib.Pipelines.RayTracingPipeline.AccelerationStructures;

import com.kgaft.VulkanLib.Device.Buffer.Buffer;
import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Pipelines.RayTracingPipeline.AccelerationStructures.ASTypes.AccelKHR;
import com.kgaft.VulkanLib.Pipelines.RayTracingPipeline.AccelerationStructures.ASTypes.BuildAccelerationStructure;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.PointerBuffer;
import org.lwjgl.vulkan.VkAccelerationStructureBuildGeometryInfoKHR;
import org.lwjgl.vulkan.VkAccelerationStructureCreateInfoKHR;
import org.lwjgl.vulkan.VkCommandBuffer;
import org.lwjgl.vulkan.VkMemoryBarrier;

import java.lang.instrument.IllegalClassFormatException;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

import static org.lwjgl.vulkan.VK13.*;
import static org.lwjgl.vulkan.KHRAccelerationStructure.*;

public class ASUtils {
    public static void cmdCreateBlas(LogicalDevice device, VkCommandBuffer cmdBuf,
                                     List<Integer> indices, List<BuildAccelerationStructure> buildAs,
                                     long scratchAddress, long queryPool){
        if(queryPool!=0)
            vkResetQueryPool(device.getDevice(), queryPool, 0, indices.size());

        AtomicInteger queryCnt = new AtomicInteger();
        indices.forEach(idx->{
            try {
                LwjglObject<VkAccelerationStructureCreateInfoKHR> createInfo = new LwjglObject<>(VkAccelerationStructureCreateInfoKHR.class);
                createInfo.get().type(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);
                createInfo.get().size(buildAs.get(idx).sizeInfo.get().accelerationStructureSize());
                buildAs.get(idx).as = createAcceleration(device, createInfo);
                buildAs.get(idx).buildInfo.get().dstAccelerationStructure(buildAs.get(idx).as.accelerationStructure);
                buildAs.get(idx).buildInfo.get().scratchData().deviceAddress(scratchAddress);
                PointerBuffer pb = PointerBuffer.allocateDirect(1);
                pb.put(buildAs.get(idx).rangeInfo.get());
                pb.rewind();

                LwjglObject<VkAccelerationStructureBuildGeometryInfoKHR.Buffer> bInfo = new LwjglObject<>(VkAccelerationStructureBuildGeometryInfoKHR.class, VkAccelerationStructureBuildGeometryInfoKHR.Buffer.class, 1);
                bInfo.get().put(buildAs.get(idx).buildInfo.get());
                bInfo.get().rewind();
                vkCmdBuildAccelerationStructuresKHR(cmdBuf, bInfo.get(), pb);

                LwjglObject<VkMemoryBarrier.Buffer> barrier = new LwjglObject<>(VkMemoryBarrier.class, VkMemoryBarrier.Buffer.class, 1);
                barrier.get().sType$Default();
                barrier.get().srcAccessMask(VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR);
                barrier.get().dstAccessMask(VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR);
                vkCmdPipelineBarrier(
                        cmdBuf, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                        VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, barrier.get(), null, null);
                if(queryPool!=0){
                    long[] accel = new long[]{buildAs.get(idx).buildInfo.get().dstAccelerationStructure()};
                    vkCmdWriteAccelerationStructuresPropertiesKHR(cmdBuf, accel, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, queryPool, queryCnt.getAndIncrement());
                    //The problem can be here!
                }
            } catch (Exception | VkErrorException e) {
                throw new RuntimeException(e);
            }

        });
    }
    public static AccelKHR createAcceleration(LogicalDevice device, LwjglObject<VkAccelerationStructureCreateInfoKHR> accel) throws IllegalClassFormatException, VkErrorException {
        AccelKHR result = new AccelKHR();
        result.buffer = new Buffer(device, accel.get().size(),
                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        accel.get().buffer(result.buffer.getBuffer());
        long[] res = new long[1];
        VkErrorException.checkVkStatus("Failed to create acceleration structure: ",
                vkCreateAccelerationStructureKHR(device.getDevice(), accel.get(), null, res));
        result.accelerationStructure = res[0];
        return result;
    }
}
