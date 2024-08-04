package com.kgaft.VulkanLib.Pipelines.PipelineConfiguration.PipelineBuilder;

public class SamplerInfo {
    public int binding;
    public int descriptorCount;
    public int shaderStages;

    public SamplerInfo(int binding, int descriptorCount, int shaderStages) {
        this.binding = binding;
        this.descriptorCount = descriptorCount;
        this.shaderStages = shaderStages;
    }

    public SamplerInfo() {
    }
}
