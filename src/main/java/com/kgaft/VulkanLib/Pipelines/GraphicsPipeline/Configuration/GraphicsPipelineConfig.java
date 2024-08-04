package com.kgaft.VulkanLib.Pipelines.GraphicsPipeline.Configuration;

import com.kgaft.VulkanLib.Utils.LwjglObject;
import org.lwjgl.vulkan.*;

import java.lang.instrument.IllegalClassFormatException;
import static org.lwjgl.vulkan.VK13.*;
public class GraphicsPipelineConfig {
    
    public static void createConfig(GraphicsPipelineConfig configInfo,  int attachmentCount, boolean alphaBlending,  int width,  int height) throws IllegalClassFormatException {
        configInfo.inputAssemblyInfo.get().sType$Default();
        configInfo.inputAssemblyInfo.get().topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        configInfo.inputAssemblyInfo.get().primitiveRestartEnable(false);
    
        configInfo.viewport.get().width(width);
        configInfo.viewport.get().height(height);
        configInfo.viewport.get().minDepth(0.0f);
        configInfo.viewport.get().maxDepth(1.0f);
        configInfo.scissor.get().offset().x(0);
        configInfo.scissor.get().offset().y(0);
        configInfo.scissor.get().extent().width(width);
        configInfo.scissor.get().extent().height(height);
        configInfo.rasterizationInfo.get().sType$Default();
        configInfo.rasterizationInfo.get().depthClampEnable(false);
        configInfo.rasterizationInfo.get().rasterizerDiscardEnable(false);
        configInfo.rasterizationInfo.get().polygonMode(VK_POLYGON_MODE_FILL);
        configInfo.rasterizationInfo.get().lineWidth(1.0f);
        configInfo.rasterizationInfo.get().cullMode(VK_CULL_MODE_NONE);
        configInfo.rasterizationInfo.get().frontFace(VK_FRONT_FACE_CLOCKWISE);
        configInfo.rasterizationInfo.get().depthBiasEnable(false);
      
        configInfo.multisampleInfo.get().sType$Default();
        configInfo.multisampleInfo.get().sampleShadingEnable(false);
        configInfo.multisampleInfo.get().rasterizationSamples(VK_SAMPLE_COUNT_1_BIT);
        configInfo.multisampleInfo.get().minSampleShading(1.0f);
        configInfo.multisampleInfo.get().alphaToCoverageEnable(false);
        configInfo.multisampleInfo.get().alphaToOneEnable(false);
     
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


        configInfo.colorBlendInfo.get().sType$Default();
        configInfo.colorBlendInfo.get().logicOpEnable(false);
        configInfo.colorBlendInfo.get().logicOp(VK_LOGIC_OP_COPY);
        configInfo.colorBlendInfo.get().attachmentCount(attachmentCount);
        configInfo.colorBlendInfo.get().pAttachments(configInfo.colorBlendAttachments.get());
        configInfo.colorBlendInfo.get().blendConstants().put(0);
        configInfo.colorBlendInfo.get().blendConstants().put(0);
        configInfo.colorBlendInfo.get().blendConstants().put(0);
        configInfo.colorBlendInfo.get().blendConstants().put(0);
        configInfo.colorBlendInfo.get().blendConstants().rewind();
        configInfo.depthStencilInfo.get().sType$Default();
        configInfo.depthStencilInfo.get().depthTestEnable(true);
        configInfo.depthStencilInfo.get().depthWriteEnable(true);
        configInfo.depthStencilInfo.get().depthCompareOp(VK_COMPARE_OP_LESS);
        configInfo.depthStencilInfo.get().depthBoundsTestEnable(false);
        configInfo.depthStencilInfo.get().minDepthBounds(0.0f);
        configInfo.depthStencilInfo.get().maxDepthBounds(1.0f);
        configInfo.depthStencilInfo.get().stencilTestEnable(false);

    }
    
    public LwjglObject<VkViewport> viewport = new LwjglObject<>(VkViewport.class);
    public LwjglObject<VkRect2D> scissor = new LwjglObject<>(VkRect2D.class);
    public LwjglObject<VkPipelineInputAssemblyStateCreateInfo> inputAssemblyInfo = new LwjglObject<>(VkPipelineInputAssemblyStateCreateInfo.class);
    public LwjglObject<VkPipelineRasterizationStateCreateInfo> rasterizationInfo = new LwjglObject<>(VkPipelineRasterizationStateCreateInfo.class);
    public LwjglObject<VkPipelineMultisampleStateCreateInfo> multisampleInfo = new LwjglObject<>(VkPipelineMultisampleStateCreateInfo.class);
    public LwjglObject<VkPipelineColorBlendAttachmentState.Buffer> colorBlendAttachments;
    public LwjglObject<VkPipelineColorBlendStateCreateInfo> colorBlendInfo = new LwjglObject<>(VkPipelineColorBlendStateCreateInfo.class);
    public LwjglObject<VkPipelineDepthStencilStateCreateInfo> depthStencilInfo = new LwjglObject<>(VkPipelineDepthStencilStateCreateInfo.class);

    public int subpass = 0;

    public GraphicsPipelineConfig() throws IllegalClassFormatException {
    }
}
