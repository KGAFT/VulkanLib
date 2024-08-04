package com.kgaft.VulkanLib.Pipelines.PipelineConfiguration.PipelineBuilder;

public class StorageBufferInfo {
    public int binding;
    public int descriptorCount;
    public int stageFlags;

    public StorageBufferInfo(int binding, int descriptorCount, int stageFlags) {
        this.binding = binding;
        this.descriptorCount = descriptorCount;
        this.stageFlags = stageFlags;
    }

    public StorageBufferInfo() {
    }
}
