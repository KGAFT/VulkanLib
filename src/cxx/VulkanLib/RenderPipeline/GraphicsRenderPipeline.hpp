//
// Created by kgaft on 12/31/23.
//

#ifndef VULKANLIB_RENDERPIPELINE_HPP
#define VULKANLIB_RENDERPIPELINE_HPP

#include <VulkanLib/Device/SwapChain/SwapChain.hpp>
#include <VulkanLib/Pipelines/GraphicsPipeline/GraphicsPipeline.hpp>
#include "RenderImagePool.hpp"

class RenderPipelineBuilder {
    friend class GraphicsRenderPipeline;

public:
    RenderPipelineBuilder();

private:
    GraphicsPipelineBuilder *pGraphicsPipelineBuilder;
    uint32_t attachmentsPerStepAmount = 0;
public:
    void addVertexInput(VertexInput input);

    void addPushConstantInfo(PushConstantInfo info);

    void addUniformBuffer(UniformBufferInfo info);

    void addSamplerInfo(SamplerInfo info);

    void addStorageBufferInfo(StorageBufferInfo info);

    void setAttachmentsPerStepAmount(uint32_t attachmentsPerStepAmount);

    void clear();

    virtual ~RenderPipelineBuilder();
};

class GraphicsRenderPipeline : public IDestroyableObject {
private:
    static inline std::shared_ptr<RenderImagePool> renderImages = std::make_shared<RenderImagePool>();
    static inline bool initializedRenderImagePool = false;

    static void checkPool(std::shared_ptr<LogicalDevice> device);

public:
    static void releaseRenderImagePool();

    static std::shared_ptr<RenderImagePool> getRenderImagePool(std::shared_ptr<LogicalDevice> device);

public:
    GraphicsRenderPipeline(Instance &instance, std::shared_ptr<LogicalDevice> device, RenderPipelineBuilder *pBuilder,
                           Shader *shader,
                           vk::Extent2D renderArea,
                           uint32_t maxFramesInFlight);

    GraphicsRenderPipeline(Instance &instance, std::shared_ptr<LogicalDevice> device,
                           std::shared_ptr<SwapChain> swapChain,
                           RenderPipelineBuilder *pBuilder,
                           Shader *shader,
                           vk::Extent2D renderArea,
                           uint32_t maxFramesInFlight);

private:
    std::shared_ptr<SwapChain> swapChain;
    Instance &instance;
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
    vk::Rect2D scissor;
    vk::Viewport viewport;
    std::vector<vk::ImageMemoryBarrier> barriers;

    bool firstRender = true;
    bool forSwapChain = false;
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

public:
    void destroy() override;
};


#endif //VULKANLIB_RENDERPIPELINE_HPP
