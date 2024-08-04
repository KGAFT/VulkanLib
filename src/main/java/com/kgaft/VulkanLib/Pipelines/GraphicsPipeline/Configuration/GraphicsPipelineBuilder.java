package com.kgaft.VulkanLib.Pipelines.GraphicsPipeline.Configuration;

import com.kgaft.VulkanLib.Pipelines.PipelineConfiguration.PipelineBuilder.*;

import java.util.ArrayList;
import java.util.List;

public class GraphicsPipelineBuilder {

    private PipelineBuilder pipelineBuilder;
    private List<Integer> colorAttachmentInfo = new ArrayList<>();
    private int depthAttachmentInfo;

    public void addVertexInput(VertexInput input) {
        pipelineBuilder.addVertexInput(input);
    }

    public void addStorageBufferInfo(StorageBufferInfo info){
        pipelineBuilder.addStorageBuffer(info);
    }

    public void addPushConstantInfo(PushConstantInfo info) {
        pipelineBuilder.addPushConstantInfo(info);
    }

    public void addUniformBuffer(UniformBufferInfo info) {
        pipelineBuilder.addUniformBuffer(info);
    }

    public void addSamplerInfo(SamplerInfo info) {
        pipelineBuilder.addSamplerInfo(info);
    }

    public void addColorAttachmentInfo(int colorAttachmentFormat) {
        colorAttachmentInfo.add(colorAttachmentFormat);
    }

    public void setDepthAttachmentInfo(int depthAttachmentInfo) {
        this.depthAttachmentInfo = depthAttachmentInfo;
    }
    public void addStorageImageInfo(StorageImageInfo storageImageInfo){
        pipelineBuilder.addStorageImageInfo(storageImageInfo);
    }
    
}
