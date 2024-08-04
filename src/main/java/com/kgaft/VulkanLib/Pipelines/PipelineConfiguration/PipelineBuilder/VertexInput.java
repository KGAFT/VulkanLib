package com.kgaft.VulkanLib.Pipelines.PipelineConfiguration.PipelineBuilder;

public class VertexInput {
    public int location;
    public int coordinatesAmount;
    public int typeSize;
    public int format;

    public VertexInput(int location, int coordinatesAmount, int typeSize, int format) {
        this.location = location;
        this.coordinatesAmount = coordinatesAmount;
        this.typeSize = typeSize;
        this.format = format;
    }

    public VertexInput() {
    }
}
