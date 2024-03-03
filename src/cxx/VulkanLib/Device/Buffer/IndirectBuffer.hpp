//
// Created by kgaft on 2/2/24.
//

#ifndef VULKANRENDERENGINE_INDIRECTBUFFER_HPP
#define VULKANRENDERENGINE_INDIRECTBUFFER_HPP

#include <vulkan/vulkan.hpp>
#include "VulkanLib/MemoryUtils/SeriesObject.hpp"
#include "Buffer.hpp"

class IndirectBuffer : public IDestroyableObject{
private:
    static inline SeriesObject<vk::BufferCreateInfo> createInfos = SeriesObject<vk::BufferCreateInfo>();
public:
    IndirectBuffer(std::shared_ptr<LogicalDevice> device, uint32_t structuresAmount, size_t sizeOfStructure) : structuresAmount(structuresAmount), sizeOfStructure(sizeOfStructure) {


        auto createInfo = createInfos.getObjectInstance();
        createInfo->sType = vk::StructureType::eBufferCreateInfo;
        createInfo->size = structuresAmount * sizeOfStructure;
        createInfo->usage = vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eTransferDst |
                            vk::BufferUsageFlagBits::eTransferSrc;
        createInfo->sharingMode = vk::SharingMode::eExclusive;
        buffer = std::make_shared<Buffer>(device, createInfo, vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);
        createInfos.releaseObjectInstance(createInfo);
        buffer->map(&mapPoint, 0, vk::MemoryMapFlags());
    }
private:
    std::shared_ptr<Buffer> buffer;
    void* mapPoint;
    uint32_t structuresAmount;
    size_t sizeOfStructure;
public:
    void *getMapPoint()  {
        return mapPoint;
    }
    vk::Buffer& getBuffer(){
        return buffer->getBuffer();
    }

    uint32_t getStructuresAmount() const {
        return structuresAmount;
    }

    size_t getSizeOfStructure() const {
        return sizeOfStructure;
    }

    void destroy() override {
        destroyed = true;
        buffer->unMap();
        buffer->destroy();
    }
};


#endif //VULKANRENDERENGINE_INDIRECTBUFFER_HPP
