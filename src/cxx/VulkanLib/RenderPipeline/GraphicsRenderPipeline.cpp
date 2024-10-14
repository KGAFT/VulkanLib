//
// Created by kgaft on 10/14/24.
//
#include "GraphicsRenderPipeline.hpp"

RenderPipelineBuilder::RenderPipelineBuilder() {
    pGraphicsPipelineBuilder = new GraphicsPipelineBuilder;
}

void RenderPipelineBuilder::addVertexInput(VertexInput input) {
    pGraphicsPipelineBuilder->addVertexInput(input);
}

void RenderPipelineBuilder::addPushConstantInfo(PushConstantInfo info) {
    pGraphicsPipelineBuilder->addPushConstantInfo(info);
}

void RenderPipelineBuilder::addUniformBuffer(UniformBufferInfo info) {
    pGraphicsPipelineBuilder->addUniformBuffer(info);
}

void RenderPipelineBuilder::addSamplerInfo(SamplerInfo info) {
    pGraphicsPipelineBuilder->addSamplerInfo(info);
}

void RenderPipelineBuilder::addStorageBufferInfo(StorageBufferInfo info) {
    pGraphicsPipelineBuilder->addStorageBufferInfo(info);
}

void RenderPipelineBuilder::setAttachmentsPerStepAmount(uint32_t attachmentsPerStepAmount) {
    RenderPipelineBuilder::attachmentsPerStepAmount = attachmentsPerStepAmount;
}

void RenderPipelineBuilder::clear() {
    delete pGraphicsPipelineBuilder;
    RenderPipelineBuilder::pGraphicsPipelineBuilder = new GraphicsPipelineBuilder;
    attachmentsPerStepAmount = 0;
}

RenderPipelineBuilder::~RenderPipelineBuilder() {
    delete pGraphicsPipelineBuilder;
}

void GraphicsRenderPipeline::checkPool(std::shared_ptr<LogicalDevice> device) {
    if (!initializedRenderImagePool) {
        renderImages = std::make_shared<RenderImagePool>(device);
        initializedRenderImagePool = true;
    }
}

void GraphicsRenderPipeline::releaseRenderImagePool() {
    renderImages->destroy();
    initializedRenderImagePool = false;
}

std::shared_ptr<RenderImagePool> GraphicsRenderPipeline::getRenderImagePool(std::shared_ptr<LogicalDevice> device) {
    checkPool(device);
    return renderImages;
}

GraphicsRenderPipeline::GraphicsRenderPipeline(Instance &instance, std::shared_ptr<LogicalDevice> device,
    RenderPipelineBuilder *pBuilder, Shader *shader, vk::Extent2D renderArea, uint32_t maxFramesInFlight): instance(instance), imagePerStepAmount(pBuilder->attachmentsPerStepAmount), renderArea(renderArea) {
    checkPool(device);
    bool populated = false;
    for (uint32_t i = 0; i < maxFramesInFlight; ++i) {
        for (uint32_t ii = 0; ii < pBuilder->attachmentsPerStepAmount; ++ii) {
            auto colorImage = renderImages->acquireColorRenderImage(renderArea.width, renderArea.height);
            baseRenderImages.push_back(colorImage);
            if (!populated)
                pBuilder->pGraphicsPipelineBuilder->addColorAttachmentInfo(colorImage->getImageInfo().format);
        }
        auto depthImage = renderImages->acquireDepthImage(renderArea.width, renderArea.height);
        baseDepthImages.push_back(depthImage);
        pBuilder->pGraphicsPipelineBuilder->setDepthAttachmentInfo(depthImage->getImageInfo().format);
        populated = true;
        depthImage->transitionImageLayout(device, vk::ImageLayout::eUndefined,
                                          vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                          vk::ImageAspectFlagBits::eDepth);
    }
    graphicsPipeline = std::make_shared<GraphicsPipeline>(*device, shader, pBuilder->pGraphicsPipelineBuilder,
                                                          pBuilder->attachmentsPerStepAmount, renderArea.width,
                                                          renderArea.height);
    depthClear.depthStencil.depth = 1.0f;
    depthClear.depthStencil.stencil = 0;
    colorClear.color = {0.0f, 0.0f, 0.0f, 1.0f};
    viewport.maxDepth = 1.0f;
    viewport.width = renderArea.width;
    viewport.height = renderArea.height;
    scissor.extent.width = renderArea.width;
    scissor.extent.height = renderArea.height;
    createImagesAndRenderingInfos(pBuilder->attachmentsPerStepAmount);

}

GraphicsRenderPipeline::GraphicsRenderPipeline(Instance &instance, std::shared_ptr<LogicalDevice> device,
    std::shared_ptr<SwapChain> swapChain, RenderPipelineBuilder *pBuilder, Shader *shader, vk::Extent2D renderArea,
    uint32_t maxFramesInFlight):
    swapChain(swapChain), instance(instance), imagePerStepAmount(1), renderArea(renderArea),
    forSwapChain(true) {
    checkPool(device);

    pBuilder->pGraphicsPipelineBuilder->addColorAttachmentInfo(swapChain->getFormat().format);
    for (uint32_t i = 0; i < maxFramesInFlight; ++i) {
        auto depthImage = renderImages->acquireDepthImage(renderArea.width, renderArea.height);
        pBuilder->pGraphicsPipelineBuilder->setDepthAttachmentInfo(depthImage->getImageInfo().format);
        baseDepthImages.push_back(depthImage);

    }
    graphicsPipeline = std::make_shared<GraphicsPipeline>(*device, shader, pBuilder->pGraphicsPipelineBuilder,
                                                          1, renderArea.width,
                                                          renderArea.height);
    depthClear.depthStencil.depth = 1.0f;
    depthClear.depthStencil.stencil = 0;
    colorClear.color = {0.0f, 0.0f, 0.0f, 1.0f};
    viewport.width = renderArea.width;
    viewport.height = renderArea.height;
    scissor.extent.width = renderArea.width;
    scissor.extent.height = renderArea.height;
    createImagesAndRenderingInfos(1);
}

void GraphicsRenderPipeline::begin(vk::CommandBuffer cmd, uint32_t currentImage) {
    prepareBarriersBeforeRendering(currentImage);
    bindBarriers(cmd);
    depthInfo.imageView = baseDepthImages[currentImage]->getImageViews()[0]->getBase();
    if (forSwapChain) {
        colorInfos[0].imageView = swapChain->getSwapchainImageViews()[currentImage]->getBase();
    } else {
        for (uint32_t i = currentImage * imagePerStepAmount; i < (currentImage + 1) * imagePerStepAmount; i++) {
            uint32_t infoIndex = i - currentImage * imagePerStepAmount;
            colorInfos[infoIndex].imageView = baseRenderImages[i]->getImageViews()[0]->getBase();
        }
    }
    cmd.beginRenderingKHR(renderingInfoKhr, instance.getDynamicLoader());
    cmd.setViewport(0, 1, &viewport, instance.getDynamicLoader());
    cmd.setScissor(0, 1, &scissor, instance.getDynamicLoader());
    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline->getGraphicsPipeline());

}

void GraphicsRenderPipeline::endRender(vk::CommandBuffer cmd, uint32_t currentImage) {
    cmd.endRenderingKHR(instance.getDynamicLoader());
    prepareBarriersAfterRendering(currentImage);
    bindBarriers(cmd);
}

void GraphicsRenderPipeline::resize(uint32_t width, uint32_t height) {
    d = false;
    firstRender = true;
    renderArea.width = width;
    renderArea.height = height;
    renderingInfoKhr.renderArea.extent.width = width;
    renderingInfoKhr.renderArea.extent.height = height;
    scissor.extent.width = width;
    scissor.extent.height = height;
    viewport.width = renderArea.width;
    viewport.height = renderArea.height;
    if (!forSwapChain)  {
        for (auto &item: baseRenderImages) {
            item->resize(width, height);
        }
    }
    for (auto &item: baseDepthImages) {
        item->resize(width, height);
    }

}

const std::vector<std::shared_ptr<Image>> & GraphicsRenderPipeline::getBaseRenderImages() const {
    return baseRenderImages;
}

const std::vector<std::shared_ptr<Image>> & GraphicsRenderPipeline::getBaseDepthImages() const {
    return baseDepthImages;
}

const std::shared_ptr<GraphicsPipeline> & GraphicsRenderPipeline::getGraphicsPipeline() const {
    return graphicsPipeline;
}

void GraphicsRenderPipeline::bindBarriers(vk::CommandBuffer cmd) {
    for (const auto &item: barriers) {
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                            vk::PipelineStageFlagBits::eTopOfPipe,
                            vk::DependencyFlags(), 0, 0,
                            item
        );
    }
}

void GraphicsRenderPipeline::createImagesAndRenderingInfos(uint32_t imagePerStepAmount) {
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
    renderingInfoKhr.colorAttachmentCount = colorInfos.size();
    renderingInfoKhr.pColorAttachments = colorInfos.data();
    renderingInfoKhr.pDepthAttachment = &depthInfo;

    barriers.resize(imagePerStepAmount);

}

void GraphicsRenderPipeline::prepareBarriersBeforeRendering(uint32_t currentImage) {
    if (firstRender) firstRender = !(d && currentImage == 0);
    for (uint32_t i = currentImage * imagePerStepAmount; i < (currentImage + 1) * imagePerStepAmount; i++) {
        uint32_t barIndex = i - currentImage * imagePerStepAmount;
        barriers[barIndex].sType = vk::StructureType::eImageMemoryBarrier;
        barriers[barIndex].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barriers[barIndex].oldLayout = firstRender ? (forSwapChain ? vk::ImageLayout::eUndefined
                                                          : vk::ImageLayout::eGeneral) : (forSwapChain
                                               ? vk::ImageLayout::ePresentSrcKHR
                                               : vk::ImageLayout::eGeneral);
        barriers[barIndex].newLayout = vk::ImageLayout::eColorAttachmentOptimal;
        barriers[barIndex].image = forSwapChain ? swapChain->getSwapchainImages()[i]->getBase()
                                       : baseRenderImages[i]->getBase();

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

void GraphicsRenderPipeline::prepareBarriersAfterRendering(uint32_t currentImage) {
    for (uint32_t i = currentImage * imagePerStepAmount; i < (currentImage + 1) * imagePerStepAmount; i++) {
        uint32_t barIndex = i - currentImage * imagePerStepAmount;
        barriers[barIndex].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barriers[barIndex].oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
        barriers[barIndex].newLayout = forSwapChain ? vk::ImageLayout::ePresentSrcKHR : vk::ImageLayout::eGeneral;
        barriers[barIndex].image = forSwapChain ? swapChain->getSwapchainImages()[i]->getBase()
                                       : baseRenderImages[i]->getBase();

        barriers[barIndex].subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };

    }
}

void GraphicsRenderPipeline::destroy() {
    destroyed = true;
    if (!forSwapChain) {
        for (auto &item: baseRenderImages) {
            renderImages->releaseColorImage(item);
        }
    }
    for (auto &item: baseDepthImages) {
        renderImages->releaseDepthImage(item);
    }
    graphicsPipeline->destroy();
}
