package com.kgaft.VulkanLib.Pipelines.PipelineConfiguration;

import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Pipelines.PipelineConfiguration.PipelineBuilder.*;
import com.kgaft.VulkanLib.Utils.DestroyableObject;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.vulkan.*;

import java.lang.instrument.IllegalClassFormatException;
import java.nio.LongBuffer;

import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

import static org.lwjgl.vulkan.KHRAccelerationStructure.*;
import static org.lwjgl.vulkan.VK13.*;

public class PipelineConfigurer extends DestroyableObject {

    private long pipelineLayout;
    private long descriptorSetLayout;
    private LogicalDevice device;
    private LwjglObject<VkVertexInputBindingDescription.Buffer> inputBindDesc = new LwjglObject<>(VkVertexInputBindingDescription.class, VkVertexInputBindingDescription.Buffer.class, 1);
    private LwjglObject<VkVertexInputAttributeDescription.Buffer> inputAttribDescs;


    public PipelineConfigurer(LogicalDevice device, PipelineBuilder builder) throws IllegalClassFormatException, VkErrorException {
        this.device = device;
        loadDescriptorSetLayout(builder);
        loadPipelineLayout(builder);
        prepareBinding(builder.getVertexInputs());
        prepareInputAttribs(builder.getVertexInputs());
    }

    public long getPipelineLayout() {
        return pipelineLayout;
    }

    public long getDescriptorSetLayout() {
        return descriptorSetLayout;
    }

    public LogicalDevice getDevice() {
        return device;
    }

    public LwjglObject<VkVertexInputBindingDescription.Buffer> getInputBindDesc() {
        return inputBindDesc;
    }

    public LwjglObject<VkVertexInputAttributeDescription.Buffer> getInputAttribDescs() {
        return inputAttribDescs;
    }

    private void loadDescriptorSetLayout(PipelineBuilder endConfiguration) throws IllegalClassFormatException, VkErrorException {
        int capacity = endConfiguration.getUniformBufferInfo().size() +
                endConfiguration.getSamplersInfo().size() +
                endConfiguration.getStorageImagesInfos().size() +
                endConfiguration.getAccelerationStructuresInfos().size() +
                endConfiguration.getStorageBufferInfos().size();
        if (capacity != 0) {
            LwjglObject<VkDescriptorSetLayoutBinding.Buffer> bindings = new LwjglObject<>(VkDescriptorSetLayoutBinding.class, VkDescriptorSetLayoutBinding.Buffer.class,
                    capacity);

            int counter = 0;
            for (UniformBufferInfo uniformBufferInfo : endConfiguration.getUniformBufferInfo()) {
                uboToBind(uniformBufferInfo, bindings.get().get(counter));
                counter++;
            }
            for (SamplerInfo samplerInfo : endConfiguration.getSamplersInfo()) {
                samplerToBind(samplerInfo, bindings.get().get(counter));
                counter++;
            }
            for (AccelerationStructureInfo accelerationStructuresInfo : endConfiguration.getAccelerationStructuresInfos()) {
                asToBind(accelerationStructuresInfo, bindings.get().get(counter));
                counter++;
            }
            for (StorageImageInfo storageImagesInfo : endConfiguration.getStorageImagesInfos()) {
                storageImageToBind(storageImagesInfo, bindings.get().get(counter));
                counter++;
            }
            for (StorageBufferInfo storageBufferInfo : endConfiguration.getStorageBufferInfos()) {
                sboToBind(storageBufferInfo, bindings.get().get(counter));
                counter++;
            }
            LwjglObject<VkDescriptorSetLayoutCreateInfo> layoutInfo = new LwjglObject<>(VkDescriptorSetLayoutCreateInfo.class);
            layoutInfo.get().pBindings(bindings.get());
            long[] res = new long[1];
            VkErrorException.checkVkStatus("Failed to create descriptor set layout: ", vkCreateDescriptorSetLayout(device.getDevice(), layoutInfo.get(), null, res));
            this.descriptorSetLayout = res[0];
        }

    }

    private void loadPipelineLayout(PipelineBuilder endConfiguration) throws IllegalClassFormatException, VkErrorException {
        LwjglObject<VkPushConstantRange.Buffer> pushConstantRanges = new LwjglObject<>(VkPushConstantRange.class, VkPushConstantRange.Buffer.class, endConfiguration.getPushConstantInfos().size());

        int counter = 0;
        for (PushConstantInfo pushConstantInfo : endConfiguration.getPushConstantInfos()) {
            infoToRange(pushConstantInfo, pushConstantRanges.get().get(counter));
            counter++;
        }
        LwjglObject<VkPipelineLayoutCreateInfo> pipelineLayoutInfo = new LwjglObject<>(VkPipelineLayoutCreateInfo.class);
        LongBuffer layouts = LongBuffer.allocate(1);
        if (descriptorSetLayout != 0) {
            layouts.put(descriptorSetLayout);
            layouts.rewind();
            pipelineLayoutInfo.get().pSetLayouts(layouts);
        }

        pipelineLayoutInfo.get().pPushConstantRanges(pushConstantRanges.get());
        long[] layout = new long[1];
        VkErrorException.checkVkStatus("Failed to create pipeline layout: ", vkCreatePipelineLayout(device.getDevice(), pipelineLayoutInfo.get(), null, layout));
    }

    private void prepareBinding(List<VertexInput> inputs) {
        AtomicInteger size = new AtomicInteger();
        inputs.forEach(element -> {
            size.getAndAdd(element.typeSize * element.coordinatesAmount);
        });
        inputBindDesc.get().binding(0);
        inputBindDesc.get().stride(size.get());
        inputBindDesc.get().inputRate(VK_VERTEX_INPUT_RATE_VERTEX);
    }

    private void prepareInputAttribs(List<VertexInput> inputs) throws IllegalClassFormatException {
        int offsetCount = 0;
        int counter = 0;
        inputAttribDescs = new LwjglObject<>(VkVertexInputAttributeDescription.class, VkVertexInputAttributeDescription.Buffer.class, inputs.size());
        for (VertexInput element : inputs) {
            inputAttribDescs.get().get(counter).binding(0);
            inputAttribDescs.get().get(counter).location(element.location);
            inputAttribDescs.get().get(counter).offset(offsetCount);
            inputAttribDescs.get().get(counter).format(element.format);
            offsetCount += element.typeSize * element.coordinatesAmount;
            counter++;
        }
    }


    private void infoToRange(PushConstantInfo info, VkPushConstantRange range) {
        range.size(info.size);
        range.offset(0);
        range.stageFlags(info.shaderStages);
    }

    private void uboToBind(UniformBufferInfo bufferInfo, VkDescriptorSetLayoutBinding result) {
        result.binding(bufferInfo.binding);
        result.descriptorType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        result.descriptorCount(bufferInfo.descriptorCount);
        result.stageFlags(bufferInfo.shaderStages);
    }

    private void sboToBind(StorageBufferInfo bufferInfo, VkDescriptorSetLayoutBinding result) {
        result.binding(bufferInfo.binding);
        result.descriptorType(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        result.descriptorCount(bufferInfo.descriptorCount);
        result.stageFlags(bufferInfo.stageFlags);
    }

    private void samplerToBind(SamplerInfo samplerInfo, VkDescriptorSetLayoutBinding result) {
        result.binding(samplerInfo.binding);
        result.descriptorType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        result.descriptorCount(samplerInfo.descriptorCount);
        result.stageFlags(samplerInfo.shaderStages);
    }

    private void asToBind(AccelerationStructureInfo asInfo, VkDescriptorSetLayoutBinding result) {
        result.binding(asInfo.binding);
        result.descriptorType(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR);
        result.descriptorCount(asInfo.descriptorCount);
        result.stageFlags(asInfo.shaderStages);
    }

    private void storageImageToBind(StorageImageInfo storageImageInfo, VkDescriptorSetLayoutBinding result) {
        result.binding(storageImageInfo.binding);
        result.descriptorType(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        result.descriptorCount(storageImageInfo.descriptorCount);
        result.stageFlags(storageImageInfo.shaderStages);
    }

    @Override
    public void destroy() {
        destroyed = true;
        vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, null);
        vkDestroyDescriptorSetLayout(device.getDevice(), descriptorSetLayout, null);
    }
}
