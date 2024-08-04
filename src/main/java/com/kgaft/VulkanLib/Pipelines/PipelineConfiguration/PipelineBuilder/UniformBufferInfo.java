package com.kgaft.VulkanLib.Pipelines.PipelineConfiguration.PipelineBuilder;

public class UniformBufferInfo {
    public int binding;
    public long size;
    public int descriptorCount;
    public int shaderStages;

    public UniformBufferInfo(int binding, long size, int descriptorCount, int shaderStages) {
        this.binding = binding;
        this.size = size;
        this.descriptorCount = descriptorCount;
        this.shaderStages = shaderStages;
    }

    public UniformBufferInfo() {
    }
}
