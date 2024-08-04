package com.kgaft.VulkanLib.Pipelines.PipelineConfiguration.PipelineBuilder;

public class PushConstantInfo {
    public int shaderStages;
    public int size;

    public PushConstantInfo(int shaderStages, int size) {
        this.shaderStages = shaderStages;
        this.size = size;
    }

    public PushConstantInfo() {
    }
}
