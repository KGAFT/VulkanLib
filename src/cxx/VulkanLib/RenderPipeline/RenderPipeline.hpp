//
// Created by kgaft on 12/31/23.
//

#ifndef VULKANLIB_RENDERPIPELINE_HPP
#define VULKANLIB_RENDERPIPELINE_HPP

#include <VulkanLib/Device/SwapChain/SwapChain.hpp>
#include <VulkanLib/GraphicsPipeline/GraphicsPipeline.hpp>
#include "RenderImagePool.hpp"

class RenderPipelineBuilder {
    friend class RenderPipeline;

public:
    RenderPipelineBuilder() {
        pGraphicsPipelineBuilder = GraphicsPipelineBuilder::getInstance();
    }

private:
    GraphicsPipelineBuilder *pGraphicsPipelineBuilder;
    uint32_t attachmentsPerStepAmount = 0;
public:
    void addVertexInput(VertexInput input) {
        pGraphicsPipelineBuilder->addVertexInput(input);
    }

    void addPushConstantInfo(PushConstantInfo info) {
        pGraphicsPipelineBuilder->addPushConstantInfo(info);
    }

    void addUniformBuffer(UniformBufferInfo info) {
        pGraphicsPipelineBuilder->addUniformBuffer(info);
    }

    void addSamplerInfo(SamplerInfo info) {
        pGraphicsPipelineBuilder->addSamplerInfo(info);
    }

    void setAttachmentsPerStepAmount(uint32_t attachmentsPerStepAmount) {
        RenderPipelineBuilder::attachmentsPerStepAmount = attachmentsPerStepAmount;
    }

};

class RenderPipeline {
private:
    static inline std::shared_ptr<RenderImagePool> renderImages = std::make_shared<RenderImagePool>();
public:
    RenderPipeline(Instance& instance, std::shared_ptr<LogicalDevice> device, RenderPipelineBuilder *pBuilder,
                   Shader *shader,
                   vk::Extent2D renderArea,
                   uint32_t maxFramesInFlight) : instance(instance), renderArea(renderArea),
                                                 imagePerStepAmount(pBuilder->attachmentsPerStepAmount) {
        bool populated = false;
        for (uint32_t i = 0; i < maxFramesInFlight; ++i) {
            for (uint32_t ii = 0; ii < pBuilder->attachmentsPerStepAmount; ++ii) {
                auto colorImage = renderImages->acquireColorRenderImage(renderArea.width, renderArea.height);
                baseRenderImages.push_back(colorImage);
                if (!populated)
                    pBuilder->pGraphicsPipelineBuilder->addColorAttachmentInfo(colorImage->getImageInfo().format);
            }
            auto depthImage = renderImages->acquiredDepthImage(renderArea.width, renderArea.height);
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
        colorClear.color = {1.0f, 0.0f, 0.0f, 1.0f};
        createImagesAndRenderingInfos(pBuilder->attachmentsPerStepAmount);

    }

    RenderPipeline(Instance& instance, std::shared_ptr<LogicalDevice> device, std::shared_ptr<SwapChain> swapChain,
                   RenderPipelineBuilder *pBuilder,
                   Shader *shader,
                   vk::Extent2D renderArea,
                   uint32_t maxFramesInFlight) : instance(instance), renderArea(renderArea), imagePerStepAmount(1), forSwapChain(true),
                                                 swapChain(swapChain) {
        pBuilder->pGraphicsPipelineBuilder->addColorAttachmentInfo(swapChain->getFormat().format);
        for (int i = 0; i < maxFramesInFlight; ++i) {
            auto depthImage = renderImages->acquiredDepthImage(renderArea.width, renderArea.height);
            pBuilder->pGraphicsPipelineBuilder->setDepthAttachmentInfo(depthImage->getImageInfo().format);
            baseDepthImages.push_back(depthImage);
            depthImage->transitionImageLayout(device, vk::ImageLayout::eUndefined,
                                              vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                              vk::ImageAspectFlagBits::eDepth);
        }
        graphicsPipeline = std::make_shared<GraphicsPipeline>(*device, shader, pBuilder->pGraphicsPipelineBuilder,
                                                              1, renderArea.width,
                                                              renderArea.height);
        depthClear.depthStencil.depth = 1.0f;
        depthClear.depthStencil.stencil = 0;
        colorClear.color = {1.0f, 0.0f, 0.0f, 1.0f};
        createImagesAndRenderingInfos(1);
    }

private:
    std::shared_ptr<SwapChain> swapChain;
    Instance& instance;
    std::vector<std::shared_ptr<Image>> baseRenderImages;
    std::vector<std::shared_ptr<Image>> baseDepthImages;
    std::shared_ptr<GraphicsPipeline> graphicsPipeline;

    std::vector<vk::RenderingAttachmentInfoKHR> colorInfos;
    vk::RenderingAttachmentInfoKHR depthInfo;
    vk::ClearValue colorClear{};
    vk::ClearValue depthClear{};
    vk::RenderingInfoKHR renderingInfoKhr{};

    uint32_t imagePerStepAmount;

    vk::Extent2D renderArea;

    std::vector<vk::ImageMemoryBarrier> barriers;

    bool firstRender = true;
    bool forSwapChain = false;
    bool d = false;
public:
    void begin(vk::CommandBuffer cmd, uint32_t currentImage){
        prepareBarriersBeforeRendering(currentImage);
        bindBarriers(cmd);
        depthInfo.imageView = baseDepthImages[currentImage]->getImageViews()[0]->getBase();
        if(forSwapChain){
            colorInfos[0].imageView = swapChain->getSwapchainImageViews()[currentImage]->getBase();
        }
        else{
            for(uint32_t i = currentImage*imagePerStepAmount; i<(currentImage+1)*imagePerStepAmount; i++){
                uint32_t infoIndex = i - currentImage * imagePerStepAmount;
                colorInfos[infoIndex].imageView = baseRenderImages[i]->getImageViews()[0]->getBase();
            }
        }
        cmd.beginRenderingKHR(renderingInfoKhr, instance.getDynamicLoader());
        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline->getGraphicsPipeline());

    }
    void endRender(vk::CommandBuffer cmd, uint32_t currentImage){
        cmd.endRenderingKHR();
        prepareBarriersAfterRendering(currentImage);
        bindBarriers(cmd);
    }
private:

    void bindBarriers(vk::CommandBuffer cmd){
        for (const auto &item: barriers){
            cmd.pipelineBarrier(    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                    vk::PipelineStageFlagBits::eTopOfPipe,
                                    vk::DependencyFlags (), 0, 0,
                                    item
            );
        }
    }

    void createImagesAndRenderingInfos(uint32_t imagePerStepAmount) {
        colorInfos.resize(imagePerStepAmount);
        for (auto &item: colorInfos) {
            item.sType = vk::StructureType::eRenderingAttachmentInfoKHR;
            item.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
            item.loadOp = vk::AttachmentLoadOp::eClear;
            item.storeOp = vk::AttachmentStoreOp::eStore;
        }
        renderingInfoKhr.renderArea = {0, 0, renderArea.width, renderArea.height};
        renderingInfoKhr.layerCount = 1;
        renderingInfoKhr.colorAttachmentCount = colorInfos.size();
        renderingInfoKhr.pColorAttachments = colorInfos.data();
        renderingInfoKhr.pDepthAttachment = &depthInfo;

        barriers.resize(imagePerStepAmount);

    }

    void prepareBarriersBeforeRendering(uint32_t currentImage) {
        if(firstRender) firstRender = !(d&&currentImage==0);
        for (uint32_t i = currentImage * imagePerStepAmount; i < (currentImage + 1) * imagePerStepAmount; i++) {
            uint32_t barIndex = i - currentImage * imagePerStepAmount;
            barriers[barIndex].sType = vk::StructureType::eImageMemoryBarrier;
            barriers[barIndex].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
            barriers[barIndex].oldLayout = firstRender?vk::ImageLayout::eUndefined:(forSwapChain?vk::ImageLayout::ePresentSrcKHR:vk::ImageLayout::eGeneral);
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

    void prepareBarriersAfterRendering(uint32_t currentImage) {
        for (uint32_t i = currentImage * imagePerStepAmount; i < (currentImage + 1) * imagePerStepAmount; i++) {
            uint32_t barIndex = i - currentImage * imagePerStepAmount;
            barriers[barIndex].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
            barriers[barIndex].oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
            barriers[barIndex].newLayout = forSwapChain?vk::ImageLayout::ePresentSrcKHR:vk::ImageLayout::eGeneral;
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
};


#endif //VULKANLIB_RENDERPIPELINE_HPP
