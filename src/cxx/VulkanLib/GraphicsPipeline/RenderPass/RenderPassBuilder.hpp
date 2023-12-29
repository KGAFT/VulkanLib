//
// Created by kgaft on 12/3/23.
//

#include <vulkan/vulkan.hpp>

struct SubPass{
    std::vector<vk::AttachmentDescription> inputAttachments;
    std::vector<vk::AttachmentDescription> outputAttachments;
    vk::AttachmentDescription depthOutDescription;
};

class RenderPassBuilder {
    friend class RenderPass;
    friend class FrameBufferManager;
private:
    std::vector<SubPass> subPasses;
public:
    void addSubPassInfo(SubPass subPass){
        subPasses.push_back(subPass);
    }
};


