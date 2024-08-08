package com.kgaft.VulkanLib.RenderPipeline;

import com.kgaft.VulkanLib.Device.Image.Image;
import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Device.SwapChain;
import com.kgaft.VulkanLib.Pipelines.GraphicsPipeline.Configuration.GraphicsPipelineBuilder;
import com.kgaft.VulkanLib.Pipelines.GraphicsPipeline.GraphicsPipeline;
import com.kgaft.VulkanLib.Shader.Shader;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.vulkan.*;

import java.lang.instrument.IllegalClassFormatException;
import java.util.ArrayList;
import java.util.List;

import static org.lwjgl.vulkan.KHRSwapchain.VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
import static org.lwjgl.vulkan.VK13.*;
import static org.lwjgl.vulkan.KHRDynamicRendering.*;
public class GraphicsRenderPipeline {

    private static RenderImagePool imagesPool = null;

    private static void checkImagesPool(LogicalDevice device){
        if(imagesPool==null){
            imagesPool = new RenderImagePool(device);
        }
    }

    private LogicalDevice device;
    private SwapChain swapChain;
    private List<Image> baseRenderImages = new ArrayList<>();
    private List<Image> baseDepthImages = new ArrayList<>();
    private GraphicsPipeline graphicsPipeline;

    private LwjglObject<VkRenderingAttachmentInfoKHR.Buffer> colorInfos;
    private LwjglObject<VkRenderingAttachmentInfoKHR> depthInfo;
    private LwjglObject<VkClearValue> colorClear = new LwjglObject<>(VkClearValue.class);
    private LwjglObject<VkClearValue> depthClear = new LwjglObject<>(VkClearValue.class);
    private LwjglObject<VkRenderingInfoKHR> renderingInfoKhr = new LwjglObject<>(VkRenderingInfoKHR.class);

    private int imagePerStepAmount;

    private LwjglObject<VkExtent2D> renderArea;

    private LwjglObject<VkImageMemoryBarrier.Buffer> barriers;

    private boolean firstRender = true;
    private boolean forSwapChain = false;
    private  boolean d = false;

    public GraphicsRenderPipeline(LogicalDevice device, GraphicsPipelineBuilder builder,
                                  Shader shader,
                                  LwjglObject<VkExtent2D> renderArea,
                                  int maxFramesInFlight, int attachmentsPerStepAmount) throws IllegalClassFormatException, VkErrorException {
        checkImagesPool(device);
        boolean populated =false;
        this.imagePerStepAmount = attachmentsPerStepAmount;


        for (int i = 0; i < maxFramesInFlight; ++i) {
            for (int ii = 0; ii < attachmentsPerStepAmount; ++ii) {
                Image colorImage = imagesPool.acquireColorImage(renderArea.get().width(), renderArea.get().height());
                baseRenderImages.add(colorImage);
                if (!populated)
                    builder.addColorAttachmentInfo(colorImage.getImageInfo().get().format());
            }
            Image depthImage = imagesPool.acquireDepthImage(renderArea.get().width(), renderArea.get().height());
            baseDepthImages.add(depthImage);
            builder.setDepthAttachmentInfo(depthImage.getImageInfo().get().format());
            populated = true;
            depthImage.transitionImageLayout(device, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
        }
        graphicsPipeline = new GraphicsPipeline(builder, attachmentsPerStepAmount, device, shader, renderArea.get().width(), renderArea.get().height());
        this.renderArea = renderArea;

        this.depthClear.get().depthStencil().depth(1);
        this.depthClear.get().depthStencil().stencil(0);
        this.colorClear.get().color().float32(0, 0);
        this.colorClear.get().color().float32(1, 0);
        this.colorClear.get().color().float32(2, 0);
        this.colorClear.get().color().float32(3, 1);
        createImagesAndRenderingInfos(attachmentsPerStepAmount);
    }

    public GraphicsRenderPipeline(LogicalDevice device,
                           SwapChain swapChain,
                                  GraphicsPipelineBuilder pBuilder,
                           Shader shader,
                                  LwjglObject<VkExtent2D> renderArea,
                           int maxFramesInFlight) throws IllegalClassFormatException, VkErrorException {
        this.forSwapChain = true;
        this.swapChain = swapChain;
        this.imagePerStepAmount = 1;
        this.renderArea = renderArea;
        checkImagesPool(device);

        pBuilder.addColorAttachmentInfo(SwapChain.getFormat().format());

        for (int i = 0; i < maxFramesInFlight; ++i) {
            Image depthImage = imagesPool.acquireDepthImage(renderArea.get().width(), renderArea.get().height());
            pBuilder.setDepthAttachmentInfo(depthImage.getImageInfo().get().format());
            baseDepthImages.add(depthImage);

        }
        graphicsPipeline = new GraphicsPipeline(pBuilder, 1, device,
                shader, renderArea.get().width(),
                renderArea.get().height());
        this.depthClear.get().depthStencil().depth(1);
        this.depthClear.get().depthStencil().stencil(0);
        this.colorClear.get().color().float32(0, 0);
        this.colorClear.get().color().float32(1, 0);
        this.colorClear.get().color().float32(2, 0);
        this.colorClear.get().color().float32(3, 1);
        createImagesAndRenderingInfos(1);
    }


    public void begin(VkCommandBuffer cmd, int currentImage) {
        prepareBarriersBeforeRendering(currentImage);
        bindBarriers(cmd);
        colorInfos.get().rewind();
        depthInfo.get().imageView(baseDepthImages.get(currentImage).getImageViews().get(0).getImageView());
        if (forSwapChain) {
            colorInfos.get().get(0).imageView(swapChain.getSwapchainImageViews().get(currentImage).getImageView());
        } else {
            for (int i = currentImage * imagePerStepAmount; i < (currentImage + 1) * imagePerStepAmount; i++) {
                int infoIndex = i - currentImage * imagePerStepAmount;
                colorInfos.get().get(infoIndex).imageView(baseRenderImages.get(i).getImageViews().get(0).getImageView());
            }
        }
        vkCmdBeginRendering(cmd, renderingInfoKhr.get());
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.getGraphicsPipeline());
    }

    public void endRender(VkCommandBuffer cmd, int currentImage) {
        vkCmdEndRendering(cmd);
        prepareBarriersAfterRendering(currentImage);
        bindBarriers(cmd);
    }

    public void resize(int width, int height) throws IllegalClassFormatException, VkErrorException {
        d = false;
        firstRender = true;
        renderArea.get().width(width);
        renderArea.get().height(height);
        renderingInfoKhr.get().renderArea().extent().width(width);
        renderingInfoKhr.get().renderArea().extent().height(height);
        if (!forSwapChain) {
            baseRenderImages.forEach(element-> {
                try {
                    element.resize(width, height);
                } catch (VkErrorException e) {
                    throw new RuntimeException(e);
                }
            });

        }
        baseDepthImages.forEach(element-> {
            try {
                element.resize(width,height);
            } catch (VkErrorException e) {
                throw new RuntimeException(e);
            }
        });

        graphicsPipeline.recreate(width, height);

    }

    private void bindBarriers(VkCommandBuffer cmd) {
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,0,null, null, barriers.get());
    }


    private void createImagesAndRenderingInfos(int imagePerStepAmount) throws IllegalClassFormatException {
        colorInfos = new LwjglObject<>(VkRenderingAttachmentInfoKHR.class, VkRenderingAttachmentInfoKHR.Buffer.class, imagePerStepAmount);
        colorInfos.get().forEach(item->{
            item.sType$Default();
            item.imageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            item.loadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);
            item.storeOp(VK_ATTACHMENT_STORE_OP_STORE);
            item.clearValue(colorClear.get());
        });
        colorInfos.get().rewind();
        depthInfo.get().sType$Default();
        depthInfo.get().imageLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        depthInfo.get().loadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);
        depthInfo.get().storeOp(VK_ATTACHMENT_STORE_OP_STORE);
        depthInfo.get().clearValue(depthClear.get());

        renderingInfoKhr.get().renderArea().extent(renderArea.get());
        renderingInfoKhr.get().layerCount(1);

        renderingInfoKhr.get().pColorAttachments(colorInfos.get());
        renderingInfoKhr.get().pDepthAttachment(depthInfo.get());

        barriers = new LwjglObject<>(VkImageMemoryBarrier.class, VkImageMemoryBarrier.Buffer.class, imagePerStepAmount);

    }
    private void prepareBarriersBeforeRendering(int currentImage) {
        if (firstRender) firstRender = !(d && currentImage == 0);
        for (int i = currentImage * imagePerStepAmount; i < (currentImage + 1) * imagePerStepAmount; i++) {
            int barIndex = i - currentImage * imagePerStepAmount;
            barriers.get().get(barIndex).sType$Default();
            barriers.get().get(barIndex).srcAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
            barriers.get().get(barIndex).oldLayout(firstRender ? (forSwapChain ? VK_IMAGE_LAYOUT_UNDEFINED
                    : VK_IMAGE_LAYOUT_GENERAL) : (forSwapChain
                    ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_GENERAL));
            barriers.get().get(barIndex).newLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            barriers.get().get(barIndex).image(forSwapChain ? swapChain.getSwapchainImages().get(i).getBase()
                                                    : baseRenderImages.get(i).getBase());

            barriers.get().get(barIndex).subresourceRange().aspectMask(VK_IMAGE_ASPECT_COLOR_BIT);
            barriers.get().get(barIndex).subresourceRange().levelCount(1);
            barriers.get().get(barIndex).subresourceRange().layerCount(1);
        }

        d = true;
    }
    private void prepareBarriersAfterRendering(int currentImage) {
        if (firstRender) firstRender = !(d && currentImage == 0);
        for (int i = currentImage * imagePerStepAmount; i < (currentImage + 1) * imagePerStepAmount; i++) {
            int barIndex = i - currentImage * imagePerStepAmount;
            barriers.get().get(barIndex).sType$Default();
            barriers.get().get(barIndex).srcAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
            barriers.get().get(barIndex).oldLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            barriers.get().get(barIndex).newLayout(forSwapChain ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_GENERAL);
            barriers.get().get(barIndex).image(forSwapChain ? swapChain.getSwapchainImages().get(i).getBase()
                    : baseRenderImages.get(i).getBase());

            barriers.get().get(barIndex).subresourceRange().aspectMask(VK_IMAGE_ASPECT_COLOR_BIT);
            barriers.get().get(barIndex).subresourceRange().levelCount(1);
            barriers.get().get(barIndex).subresourceRange().layerCount(1);
        }

        d = true;
    }

}
