//
// Created by kgaft on 12/3/23.
//
#pragma once

#include <vulkan/vulkan.hpp>

class PushConstant {
public:
    PushConstant(size_t size, vk::PipelineLayout pipelineLayout) : size(size), layout(pipelineLayout){
        data = malloc(size);
    }
private:
    void* data;
    size_t size;
    vk::PipelineLayout layout;
public:
    void bind(vk::CommandBuffer cmd, vk::ShaderStageFlags bindPoint){
        cmd.pushConstants(layout, bindPoint, 0, size, data);
    }
    void writeData(void* data, size_t writeSize){
        if(writeSize>size){
            throw std::runtime_error("Error: you cannot write data that size exceeds the push constant size");
        }
        memcpy(PushConstant::data, data, writeSize);
    }
};


