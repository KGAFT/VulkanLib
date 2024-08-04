package com.kgaft.VulkanLib.Pipelines.GraphicsPipeline.Configuration;

import com.kgaft.VulkanLib.Utils.DestroyableObject;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import com.kgaft.VulkanLib.Utils.SeriesObject;
import org.lwjgl.vulkan.*;

import java.lang.instrument.IllegalClassFormatException;
import static org.lwjgl.vulkan.VK13.*;
public class GraphicsPipelineConfig extends DestroyableObject {

    private static SeriesObject<VkPipelineInputAssemblyStateCreateInfo> inputAssemblyInfos;
    private static SeriesObject<VkPipelineRasterizationStateCreateInfo> rasterizationInfos;
    private static SeriesObject<VkPipelineMultisampleStateCreateInfo> multisampleInfos;
    private static SeriesObject<VkPipelineColorBlendStateCreateInfo> colorBlendInfos;
    private static SeriesObject<VkPipelineDepthStencilStateCreateInfo> depthStencilInfos;
    static {
        try {
            depthStencilInfos = new SeriesObject<>(VkPipelineDepthStencilStateCreateInfo.class);
            colorBlendInfos = new SeriesObject<>(VkPipelineColorBlendStateCreateInfo.class);
            multisampleInfos = new SeriesObject<>(VkPipelineMultisampleStateCreateInfo.class);
            rasterizationInfos = new SeriesObject<>(VkPipelineRasterizationStateCreateInfo.class);
            inputAssemblyInfos = new SeriesObject<>(VkPipelineInputAssemblyStateCreateInfo.class);
        } catch (IllegalClassFormatException e) {
            throw new RuntimeException(e);
        }
    }
    public static void createConfig(GraphicsPipelineConfig configInfo,  int attachmentCount, boolean alphaBlending,  int width,  int height) throws IllegalClassFormatException {
        configInfo.inputAssemblyInfo.sType$Default();
        configInfo.inputAssemblyInfo.topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        configInfo.inputAssemblyInfo.primitiveRestartEnable(false);
    
        configInfo.viewport.get().width(width);
        configInfo.viewport.get().height(height);
        configInfo.viewport.get().minDepth(0.0f);
        configInfo.viewport.get().maxDepth(1.0f);
        configInfo.scissor.get().offset().x(0);
        configInfo.scissor.get().offset().y(0);
        configInfo.scissor.get().extent().width(width);
        configInfo.scissor.get().extent().height(height);
        configInfo.rasterizationInfo.sType$Default();
        configInfo.rasterizationInfo.depthClampEnable(false);
        configInfo.rasterizationInfo.rasterizerDiscardEnable(false);
        configInfo.rasterizationInfo.polygonMode(VK_POLYGON_MODE_FILL);
        configInfo.rasterizationInfo.lineWidth(1.0f);
        configInfo.rasterizationInfo.cullMode(VK_CULL_MODE_NONE);
        configInfo.rasterizationInfo.frontFace(VK_FRONT_FACE_CLOCKWISE);
        configInfo.rasterizationInfo.depthBiasEnable(false);
      
        configInfo.multisampleInfo.sType$Default();
        configInfo.multisampleInfo.sampleShadingEnable(false);
        configInfo.multisampleInfo.rasterizationSamples(VK_SAMPLE_COUNT_1_BIT);
        configInfo.multisampleInfo.minSampleShading(1.0f);
        configInfo.multisampleInfo.alphaToCoverageEnable(false);
        configInfo.multisampleInfo.alphaToOneEnable(false);
     
        if(alphaBlending){
            configInfo.colorBlendAttachments = new LwjglObject<>(VkPipelineColorBlendAttachmentState.class, VkPipelineColorBlendAttachmentState.Buffer.class, attachmentCount);
            for (int i = 0; i < attachmentCount; ++i){
                configInfo.colorBlendAttachments.get().get(i).blendEnable(true);
                configInfo.colorBlendAttachments.get().get(i).colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
                configInfo.colorBlendAttachments.get().get(i).srcColorBlendFactor(VK_BLEND_FACTOR_SRC_ALPHA);
                configInfo.colorBlendAttachments.get().get(i).dstColorBlendFactor(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
                configInfo.colorBlendAttachments.get().get(i).colorBlendOp(VK_BLEND_OP_ADD);
                configInfo.colorBlendAttachments.get().get(i).srcAlphaBlendFactor(VK_BLEND_FACTOR_ONE);
                configInfo.colorBlendAttachments.get().get(i).dstAlphaBlendFactor(VK_BLEND_FACTOR_ZERO);
                configInfo.colorBlendAttachments.get().get(i).alphaBlendOp(VK_BLEND_OP_ADD);
            }
        }
        else{
            for (int i = 0; i < attachmentCount; ++i){
                configInfo.colorBlendAttachments.get().get(i).colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
                configInfo.colorBlendAttachments.get().get(i).blendEnable(false);
                configInfo.colorBlendAttachments.get().get(i).srcColorBlendFactor(VK_BLEND_FACTOR_ONE);
                configInfo.colorBlendAttachments.get().get(i).dstColorBlendFactor(VK_BLEND_FACTOR_ZERO);
                configInfo.colorBlendAttachments.get().get(i).colorBlendOp(VK_BLEND_OP_ADD);
                configInfo.colorBlendAttachments.get().get(i).srcAlphaBlendFactor(VK_BLEND_FACTOR_ONE);
                configInfo.colorBlendAttachments.get().get(i).dstAlphaBlendFactor(VK_BLEND_FACTOR_ZERO);
                configInfo.colorBlendAttachments.get().get(i).alphaBlendOp(VK_BLEND_OP_ADD);
            }
        }


        configInfo.colorBlendInfo.sType$Default();
        configInfo.colorBlendInfo.logicOpEnable(false);
        configInfo.colorBlendInfo.logicOp(VK_LOGIC_OP_COPY);
        configInfo.colorBlendInfo.attachmentCount(attachmentCount);
        configInfo.colorBlendInfo.pAttachments(configInfo.colorBlendAttachments.get());
        configInfo.colorBlendInfo.blendConstants().put(0);
        configInfo.colorBlendInfo.blendConstants().put(0);
        configInfo.colorBlendInfo.blendConstants().put(0);
        configInfo.colorBlendInfo.blendConstants().put(0);
        configInfo.colorBlendInfo.blendConstants().rewind();
        configInfo.depthStencilInfo.sType$Default();
        configInfo.depthStencilInfo.depthTestEnable(true);
        configInfo.depthStencilInfo.depthWriteEnable(true);
        configInfo.depthStencilInfo.depthCompareOp(VK_COMPARE_OP_LESS);
        configInfo.depthStencilInfo.depthBoundsTestEnable(false);
        configInfo.depthStencilInfo.minDepthBounds(0.0f);
        configInfo.depthStencilInfo.maxDepthBounds(1.0f);
        configInfo.depthStencilInfo.stencilTestEnable(false);

    }




    public LwjglObject<VkViewport.Buffer> viewport = new LwjglObject<>(VkViewport.class, VkViewport.Buffer.class, 1);
    public LwjglObject<VkRect2D.Buffer> scissor = new LwjglObject<>(VkRect2D.class, VkRect2D.Buffer.class, 1);
    public VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = inputAssemblyInfos.acquireObject();
    public VkPipelineRasterizationStateCreateInfo rasterizationInfo = rasterizationInfos.acquireObject();
    public VkPipelineMultisampleStateCreateInfo multisampleInfo = multisampleInfos.acquireObject();
    public LwjglObject<VkPipelineColorBlendAttachmentState.Buffer> colorBlendAttachments;
    public VkPipelineColorBlendStateCreateInfo colorBlendInfo = colorBlendInfos.acquireObject();
    public VkPipelineDepthStencilStateCreateInfo depthStencilInfo = depthStencilInfos.acquireObject();

    public int subpass = 0;

    public GraphicsPipelineConfig() throws IllegalClassFormatException {
    }

    @Override
    public void destroy() {
        destroyed = true;
        inputAssemblyInfos.releaseObjectInstance(inputAssemblyInfo);
        rasterizationInfos.releaseObjectInstance(rasterizationInfo);
        multisampleInfos.releaseObjectInstance(multisampleInfo);
        colorBlendInfos.releaseObjectInstance(colorBlendInfo);
        depthStencilInfos.releaseObjectInstance(depthStencilInfo);
    }
}
