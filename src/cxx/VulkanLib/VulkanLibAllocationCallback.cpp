//
// Created by kgaft on 9/8/24.
//

#include "VulkanLibAllocationCallback.h"

#include <iostream>
#include <malloc.h>

namespace VkLibAlloc {
    std::shared_ptr<vk::AllocationCallbacks> acquireAllocCb() {
        return nullptr;
    }

}
