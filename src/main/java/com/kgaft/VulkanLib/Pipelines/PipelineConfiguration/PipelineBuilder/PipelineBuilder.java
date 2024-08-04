package com.kgaft.VulkanLib.Pipelines.PipelineConfiguration.PipelineBuilder;

import java.util.ArrayList;
import java.util.List;

public class PipelineBuilder {

    private List<VertexInput> vertexInputs = new ArrayList<>();
    private List<UniformBufferInfo> uniformBufferInfo = new ArrayList<>();
    private List<PushConstantInfo> pushConstantInfos = new ArrayList<>();
    private List<SamplerInfo> samplersInfo = new ArrayList<>();
    private List<AccelerationStructureInfo> accelerationStructuresInfos = new ArrayList<>();
    private List<StorageImageInfo> storageImagesInfos = new ArrayList<>();
    private List<StorageBufferInfo> storageBufferInfos = new ArrayList<>();

    public void addVertexInput(VertexInput input) {
        vertexInputs.add(input);
    }

    public void addPushConstantInfo(PushConstantInfo info) {
        pushConstantInfos.add(info);
    }

    public void addUniformBuffer(UniformBufferInfo info) {
        uniformBufferInfo.add(info);
    }

    public void addStorageBuffer(StorageBufferInfo info){
        storageBufferInfos.add(info);
    }

    public void addSamplerInfo(SamplerInfo info) {
        samplersInfo.add(info);
    }
    public void addAccelerationStructureInfo(AccelerationStructureInfo info){
        accelerationStructuresInfos.add(info);
    }
    public void addStorageImageInfo(StorageImageInfo storageImageInfo){
        storageImagesInfos.add(storageImageInfo);
    }

    public List<VertexInput> getVertexInputs() {
        return vertexInputs;
    }

    public List<UniformBufferInfo> getUniformBufferInfo() {
        return uniformBufferInfo;
    }

    public List<PushConstantInfo> getPushConstantInfos() {
        return pushConstantInfos;
    }

    public List<SamplerInfo> getSamplersInfo() {
        return samplersInfo;
    }

    public List<AccelerationStructureInfo> getAccelerationStructuresInfos() {
        return accelerationStructuresInfos;
    }

    public List<StorageImageInfo> getStorageImagesInfos() {
        return storageImagesInfos;
    }

    public List<StorageBufferInfo> getStorageBufferInfos() {
        return storageBufferInfos;
    }
}
