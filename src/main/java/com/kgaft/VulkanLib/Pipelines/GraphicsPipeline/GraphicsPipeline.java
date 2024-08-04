package com.kgaft.VulkanLib.Pipelines.GraphicsPipeline;

import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Pipelines.GraphicsPipeline.Configuration.GraphicsPipelineBuilder;
import com.kgaft.VulkanLib.Pipelines.GraphicsPipeline.Configuration.GraphicsPipelineConfig;
import com.kgaft.VulkanLib.Pipelines.PipelineConfiguration.PipelineConfigurer;
import com.kgaft.VulkanLib.Shader.Shader;
import com.kgaft.VulkanLib.Utils.DestroyableObject;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import com.kgaft.VulkanLib.Utils.SeriesObject;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.vulkan.VkGraphicsPipelineCreateInfo;
import org.lwjgl.vulkan.VkPipelineRenderingCreateInfo;
import org.lwjgl.vulkan.VkPipelineVertexInputStateCreateInfo;
import org.lwjgl.vulkan.VkPipelineViewportStateCreateInfo;

import java.lang.instrument.IllegalClassFormatException;
import java.nio.IntBuffer;
import java.util.ArrayList;
import java.util.List;
import static org.lwjgl.vulkan.VK13.*;


class GraphicsPipelineCreateStrip extends DestroyableObject {
    private static SeriesObject<VkPipelineVertexInputStateCreateInfo> vertexInputs;
    private static SeriesObject<VkPipelineViewportStateCreateInfo> viewPortsInfos;
    static {
        try {
            vertexInputs = new SeriesObject<>(VkPipelineVertexInputStateCreateInfo.class);
            viewPortsInfos = new SeriesObject<>(VkPipelineViewportStateCreateInfo.class);
        } catch (IllegalClassFormatException e) {
            throw new RuntimeException(e);
        }
    }

    public VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    public VkPipelineViewportStateCreateInfo viewportInfo;

    public LwjglObject<VkGraphicsPipelineCreateInfo.Buffer> pipelineInfo = new LwjglObject<>(VkGraphicsPipelineCreateInfo.class, VkGraphicsPipelineCreateInfo.Buffer.class, 1);

    GraphicsPipelineCreateStrip() throws IllegalClassFormatException {
        vertexInputInfo = vertexInputs.acquireObject();
        viewportInfo = viewPortsInfos.acquireObject();
        
    }

    @Override
    public void destroy() {
        destroyed = true;
        vertexInputs.releaseObjectInstance(vertexInputInfo);
        viewPortsInfos.releaseObjectInstance(viewportInfo);
        
    }
}

public class GraphicsPipeline extends DestroyableObject{


    private List<Integer> attachmentsFormats = new ArrayList<>();
    private int depthFormat;
    private PipelineConfigurer configurer;
    private long graphicsPipeline;
    private LogicalDevice device;
    private Shader shader;
    private  int attachmentPerStepAmount;

    public GraphicsPipeline(GraphicsPipelineBuilder builder, int attachmentPerStepAmount, LogicalDevice device, Shader shader, int width, int height) throws IllegalClassFormatException, VkErrorException {
        this.attachmentPerStepAmount = attachmentPerStepAmount;
        this.device = device;
        this.shader = shader;
        this.configurer = new PipelineConfigurer(device, builder.getPipelineBuilder());
        this.depthFormat = builder.getDepthAttachmentInfo();
        this.attachmentsFormats = builder.getColorAttachmentInfo();
        create(attachmentPerStepAmount, width, height, shader, attachmentsFormats, depthFormat);
    }

    public void recreate(int width, int height) throws IllegalClassFormatException, VkErrorException {
        vkDestroyPipeline(device.getDevice(), graphicsPipeline, null);
        create(attachmentPerStepAmount, width, height, shader, attachmentsFormats, depthFormat);
    }

    public long getGraphicsPipeline() {
        return graphicsPipeline;
    }

    public PipelineConfigurer getConfigurer() {
        return configurer;
    }

    private void create(int attachmentPerStepAmount, int width, int height, Shader shader,
                        List<Integer> colorFormats, int depthFormat) throws IllegalClassFormatException, VkErrorException {
        GraphicsPipelineConfig config = new GraphicsPipelineConfig();
        GraphicsPipelineCreateStrip createStrip = new GraphicsPipelineCreateStrip();
        GraphicsPipelineConfig.createConfig(config, attachmentPerStepAmount, true, width, height);

        createStrip.vertexInputInfo.sType$Default();
        createStrip.vertexInputInfo.pVertexAttributeDescriptions(configurer.getInputAttribDescs().get());

        createStrip.vertexInputInfo.pVertexBindingDescriptions(configurer.getInputBindDesc().get());

        createStrip.viewportInfo.sType$Default();

        createStrip.viewportInfo.pViewports(config.viewport.get());
        createStrip.viewportInfo.pScissors(config.scissor.get());

        createStrip.pipelineInfo.get().sType$Default();
        createStrip.pipelineInfo.get().pStages(shader.getStages().get());
        createStrip.pipelineInfo.get().pVertexInputState(createStrip.vertexInputInfo);
        createStrip.pipelineInfo.get().pInputAssemblyState(config.inputAssemblyInfo);
        createStrip.pipelineInfo.get().pViewportState(createStrip.viewportInfo);
        createStrip.pipelineInfo.get().pRasterizationState(config.rasterizationInfo);
        createStrip.pipelineInfo.get().pMultisampleState(config.multisampleInfo);
        createStrip.pipelineInfo.get().pColorBlendState(config.colorBlendInfo);
        createStrip.pipelineInfo.get().pDepthStencilState(config.depthStencilInfo);
        createStrip.pipelineInfo.get().pDynamicState(null);

        createStrip.pipelineInfo.get().layout(configurer.getPipelineLayout());
        createStrip.pipelineInfo.get().subpass(0);

        createStrip.pipelineInfo.get().basePipelineIndex(-1);
        createStrip.pipelineInfo.get().basePipelineHandle(0);

        LwjglObject<VkPipelineRenderingCreateInfo> renderingCreateInfo = new LwjglObject<>(VkPipelineRenderingCreateInfo.class);
        IntBuffer colorInfo = IntBuffer.allocate(colorFormats.size());
        colorFormats.forEach(colorInfo::put);
        colorInfo.rewind();
        renderingCreateInfo.get().pColorAttachmentFormats(colorInfo);
        renderingCreateInfo.get().depthAttachmentFormat(depthFormat);

        createStrip.pipelineInfo.get().pNext(renderingCreateInfo.get().address());
        long[] res = new long[1];
        VkErrorException.checkVkStatus("Failed to crate graphics pipeline: ", vkCreateGraphicsPipelines(device.getDevice(), 0, createStrip.pipelineInfo.get(), null, res));
        this.graphicsPipeline = res[0];
    }

    @Override
    public void destroy() {
        destroyed = true;
        vkDestroyPipeline(device.getDevice(), graphicsPipeline, null);
        configurer.destroy();
    }
}
