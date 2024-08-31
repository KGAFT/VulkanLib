#include "PushConstant.hpp"

PushConstant::PushConstant(size_t size, vk::PipelineLayout pipelineLayout) : size(size), layout(pipelineLayout){
    data = malloc(size);
}

void PushConstant::bind(vk::CommandBuffer cmd, vk::ShaderStageFlags bindPoint) {
    cmd.pushConstants(layout, bindPoint, 0, (uint32_t)size, data);
}

void PushConstant::writeData(void *pData, size_t writeSize) {
    if(writeSize>size){
        throw std::runtime_error("Error: you cannot write pData that size exceeds the push constant size");
    }
    memcpy(PushConstant::data, pData, writeSize);
}

void PushConstant::destroy() {
    destroyed = true;
    free(data);
}
