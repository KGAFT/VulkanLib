//
// Created by kgaft on 2/2/24.
//

#ifndef VULKANLIB_INDIRECTBUFFER_HPP
#define VULKANLIB_INDIRECTBUFFER_HPP

#include <vulkan/vulkan.hpp>
#include "VulkanLib/MemoryUtils/SeriesObject.hpp"
#include "Buffer.hpp"

class IndirectBuffer : public IDestroyableObject{
private:
    static inline auto createInfos = SeriesObject<vk::BufferCreateInfo>();
public:
    IndirectBuffer(std::shared_ptr<LogicalDevice> device, uint32_t structuresAmount, size_t sizeOfStructure);
private:
    std::shared_ptr<Buffer> buffer;
    void* mapPoint;
    uint32_t structuresAmount;
    size_t sizeOfStructure;
public:
    void *getMapPoint();
    vk::Buffer& getBuffer();

    uint32_t getStructuresAmount() const;

    size_t getSizeOfStructure() const;

    void destroy() override;
};


#endif //VULKANLIB_INDIRECTBUFFER_HPP
