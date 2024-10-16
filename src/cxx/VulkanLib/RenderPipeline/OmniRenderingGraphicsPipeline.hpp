//
// Created by kgaft on 10/14/24.
//

#ifndef OMNIRENDERINGGRAPHICSPIPELINE_HPP
#define OMNIRENDERINGGRAPHICSPIPELINE_HPP
#include "GraphicsRenderPipeline.hpp"
#include "VulkanLib/Pipelines/GraphicsPipeline/GraphicsPipeline.hpp"


class OmniRenderingGraphicsPipeline {
public:
    OmniRenderingGraphicsPipeline(Instance &instance, std::shared_ptr<LogicalDevice> device, RenderPipelineBuilder *pBuilder,
                       Shader *shader,
                       vk::Extent2D renderArea);
private:
    Instance instance;
    std::shared_ptr<Image> cubeTarget;
    std::shared_ptr<Image> depthTarget;
    std::shared_ptr<GraphicsPipeline> graphicsPipeline;

    std::vector<vk::RenderingAttachmentInfoKHR> colorInfos;
    vk::RenderingAttachmentInfoKHR depthInfo;
    vk::ClearValue colorClear{};
    vk::ClearValue depthClear{};
    vk::RenderingInfoKHR renderingInfoKhr{};

    uint32_t imagePerStepAmount;

    vk::Extent2D renderArea;
    vk::Rect2D scissor;
    vk::Viewport viewport;
    std::vector<vk::ImageMemoryBarrier> barriers;

    bool firstRender = true;
    bool d = false;
public:

    void begin(vk::CommandBuffer cmd, uint32_t currentImage);

    void endRender(vk::CommandBuffer cmd, uint32_t currentImage);

    void resize(uint32_t width, uint32_t height);

    const std::vector<std::shared_ptr<Image>> &getBaseRenderImages() const;

    const std::vector<std::shared_ptr<Image>> &getBaseDepthImages() const;

    const std::shared_ptr<GraphicsPipeline> &getGraphicsPipeline() const;

private:
    void bindBarriers(vk::CommandBuffer cmd);
    void createImagesAndRenderingInfos(uint32_t imagePerStepAmount);
    void prepareBarriersBeforeRendering(uint32_t currentImage);

    void prepareBarriersAfterRendering(uint32_t currentImage);
};



#endif //OMNIRENDERINGGRAPHICSPIPELINE_HPP
