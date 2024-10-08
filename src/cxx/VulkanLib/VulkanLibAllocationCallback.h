//
// Created by kgaft on 9/8/24.
//

#ifndef VULKANLIBALLOCATIONCALLBACK_H
#define VULKANLIBALLOCATIONCALLBACK_H
#include <memory>
#include <vulkan/vulkan.hpp>


namespace VkLibAlloc {

    std::shared_ptr<vk::AllocationCallbacks> acquireAllocCb();
    void enableGC(bool value);
}


#endif //VULKANLIBALLOCATIONCALLBACK_H
