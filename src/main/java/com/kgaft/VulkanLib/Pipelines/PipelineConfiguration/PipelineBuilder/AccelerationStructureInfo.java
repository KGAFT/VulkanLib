package com.kgaft.VulkanLib.Pipelines.PipelineConfiguration.PipelineBuilder;

public class AccelerationStructureInfo {
    public int binding;
    public int descriptorCount;
    public int shaderStages;

    public AccelerationStructureInfo(int binding, int descriptorCount, int shaderStages) {
        this.binding = binding;
        this.descriptorCount = descriptorCount;
        this.shaderStages = shaderStages;
    }

    public AccelerationStructureInfo() {
    }
}
