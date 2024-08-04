package com.kgaft.VulkanLib.Pipelines.PipelineConfiguration.PipelineBuilder;

public class StorageImageInfo {
    public int binding;
    public int descriptorCount;
    public int shaderStages;

    public StorageImageInfo(int binding, int descriptorCount, int shaderStages) {
        this.binding = binding;
        this.descriptorCount = descriptorCount;
        this.shaderStages = shaderStages;
    }

    public StorageImageInfo() {
    }
}
