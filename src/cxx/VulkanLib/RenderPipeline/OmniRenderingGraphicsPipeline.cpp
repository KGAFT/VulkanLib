//
// Created by kgaft on 10/14/24.
//

#include "OmniRenderingGraphicsPipeline.hpp"

OmniRenderingGraphicsPipeline::OmniRenderingGraphicsPipeline(Instance &instance, std::shared_ptr<LogicalDevice> device,
                                                             RenderPipelineBuilder *pBuilder, Shader *shader,
                                                             vk::Extent2D renderArea): instance(instance),
    imagePerStepAmount(1), renderArea(renderArea) {
    bool populated = false;

    cubeTarget = GraphicsRenderPipeline::getRenderImagePool(device)->acquireCubeImage(
        renderArea.width, renderArea.height);
    pBuilder->pGraphicsPipelineBuilder->addColorAttachmentInfo(cubeTarget->getImageInfo().format);
    pBuilder->attachmentsPerStepAmount = 1;


    depthTarget = GraphicsRenderPipeline::getRenderImagePool(device)->acquireDepthImage(
        renderArea.width, renderArea.height);
    pBuilder->pGraphicsPipelineBuilder->setDepthAttachmentInfo(depthTarget->getImageInfo().format);
    populated = true;
    depthTarget->transitionImageLayout(device, vk::ImageLayout::eUndefined,
                                       vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                       vk::ImageAspectFlagBits::eDepth);

    graphicsPipeline = std::make_shared<GraphicsPipeline>(*device, shader, pBuilder->pGraphicsPipelineBuilder,
                                                          pBuilder->attachmentsPerStepAmount, renderArea.width,
                                                          renderArea.height);
    depthClear.depthStencil.depth = 1.0f;
    depthClear.depthStencil.stencil = 0;
    colorClear.color = {0.0f, 0.0f, 0.0f, 1.0f};
    viewport.maxDepth = 1.0f;
    viewport.width = (float)renderArea.width;
    viewport.height = (float)renderArea.height;
    scissor.extent.width = renderArea.width;
    scissor.extent.height = renderArea.height;
    createImagesAndRenderingInfos(pBuilder->attachmentsPerStepAmount);
}

void OmniRenderingGraphicsPipeline::begin(vk::CommandBuffer cmd, uint32_t currentImage) {
    prepareBarriersBeforeRendering(0);
    bindBarriers(cmd);
    depthInfo.imageView = depthTarget->getImageViews()[0]->getBase();


    colorInfos[0].imageView = cubeTarget->getImageViews()[1 + currentImage]->getBase();


    cmd.beginRenderingKHR(renderingInfoKhr, instance.getDynamicLoader());
    cmd.setViewport(0, 1, &viewport, instance.getDynamicLoader());
    cmd.setScissor(0, 1, &scissor, instance.getDynamicLoader());
    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline->getGraphicsPipeline());
}

void OmniRenderingGraphicsPipeline::endRender(vk::CommandBuffer cmd, uint32_t currentImage) {
    cmd.endRenderingKHR(instance.getDynamicLoader());
    prepareBarriersAfterRendering(0);
    bindBarriers(cmd);
}

const std::shared_ptr<GraphicsPipeline> & OmniRenderingGraphicsPipeline::getGraphicsPipeline() const {
    return graphicsPipeline;
}

void OmniRenderingGraphicsPipeline::bindBarriers(vk::CommandBuffer cmd) {
    for (const auto &item: barriers) {
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                            vk::PipelineStageFlagBits::eTopOfPipe,
                            vk::DependencyFlags(), 0, 0,
                            item
        );
    }
}

void OmniRenderingGraphicsPipeline::createImagesAndRenderingInfos(uint32_t imagePerStepAmount) {
    colorInfos.resize(imagePerStepAmount);
    for (auto &item: colorInfos) {
        item.sType = vk::StructureType::eRenderingAttachmentInfoKHR;
        item.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        item.loadOp = vk::AttachmentLoadOp::eClear;
        item.storeOp = vk::AttachmentStoreOp::eStore;
        item.clearValue = colorClear;
    }
    depthInfo.sType = vk::StructureType::eRenderingAttachmentInfoKHR;
    depthInfo.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    depthInfo.loadOp = vk::AttachmentLoadOp::eClear;
    depthInfo.storeOp = vk::AttachmentStoreOp::eStore;
    depthInfo.clearValue = depthClear;

    renderingInfoKhr.renderArea = {0, 0, renderArea.width, renderArea.height};
    renderingInfoKhr.layerCount = 1;
    renderingInfoKhr.colorAttachmentCount = (uint32_t)colorInfos.size();
    renderingInfoKhr.pColorAttachments = colorInfos.data();
    renderingInfoKhr.pDepthAttachment = &depthInfo;

    barriers.resize(imagePerStepAmount);
}

void OmniRenderingGraphicsPipeline::prepareBarriersBeforeRendering(uint32_t currentImage) {
    if (firstRender) firstRender = !(d && currentImage == 0);
    for (uint32_t i = currentImage * imagePerStepAmount; i < (currentImage + 1) * imagePerStepAmount; i++) {
        uint32_t barIndex = i - currentImage * imagePerStepAmount;
        barriers[barIndex].sType = vk::StructureType::eImageMemoryBarrier;
        barriers[barIndex].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barriers[barIndex].oldLayout = vk::ImageLayout::eGeneral;
        barriers[barIndex].newLayout = vk::ImageLayout::eColorAttachmentOptimal;
        barriers[barIndex].image = cubeTarget->getBase();

        barriers[barIndex].subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
    }

    d = true;
}

void OmniRenderingGraphicsPipeline::prepareBarriersAfterRendering(uint32_t currentImage) {
    for (uint32_t i = currentImage * imagePerStepAmount; i < (currentImage + 1) * imagePerStepAmount; i++) {
        uint32_t barIndex = i - currentImage * imagePerStepAmount;
        barriers[barIndex].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barriers[barIndex].oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
        barriers[barIndex].newLayout = vk::ImageLayout::eGeneral;
        barriers[barIndex].image = cubeTarget->getBase();

        barriers[barIndex].subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
    }
}
