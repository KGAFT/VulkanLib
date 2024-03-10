//
// Created by kgaft on 12/3/23.
//
#ifndef VULKANLIB_PUSHCONSTANT_HPP
#define VULKANLIB_PUSHCONSTANT_HPP

#include <vulkan/vulkan.hpp>
#include "VulkanLib/MemoryUtils/IDestroyableObject.hpp"

class PushConstant : public IDestroyableObject {
public:
    PushConstant(size_t size, vk::PipelineLayout pipelineLayout);

private:
    void *data;
    size_t size;
    vk::PipelineLayout layout;
public:
    void bind(vk::CommandBuffer cmd, vk::ShaderStageFlags bindPoint);

    void writeData(void *pData, size_t writeSize);
public:
    void destroy() override;
};

#endif

