//
// Created by kgaft on 12/3/23.
//

#include <vulkan/vulkan.hpp>

struct AttachmentDesc{
    vk::AttachmentDescription description;
    vk::ImageLayout layout;
    //Any value
    uint32_t i;
};

struct SubPass{
    std::vector<AttachmentDesc> outputAttachments;

    AttachmentDesc outputDepthAttachment;

    std::vector<AttachmentDesc> inputAttachments;
};

class RenderPassBuilder {
    friend class RenderPass;
    std::vector<SubPass> subPasses;
};


