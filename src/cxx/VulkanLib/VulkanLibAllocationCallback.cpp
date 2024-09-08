//
// Created by kgaft on 9/8/24.
//

#include "VulkanLibAllocationCallback.h"

#include <gc.h>
#include <iostream>
#include <malloc.h>

namespace VkLibAlloc {
    std::shared_ptr<vk::AllocationCallbacks> instance = nullptr;
    void *allocationFunction(void *pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
        auto result = GC_memalign(alignment, size);
        memset(result, 0, size);
        return result;
    }


    void freeFunction(void *pUserData, void *pMemory) {
        char* viewData = (char*) pMemory;
        std::cout<<viewData<<std::endl;
        GC_free(pMemory);
    }


    void *reallocationFunction(void *pUserData, void *pOriginal, size_t size, size_t alignment,
                               VkSystemAllocationScope allocationScope) {
        return GC_realloc(pOriginal, size);
    }


    void internalAllocationNotification(void *pUserData, size_t size, VkInternalAllocationType allocationType,
                                        VkSystemAllocationScope allocationScope) {

    }


    void internalFreeNotification(void *pUserData, size_t size, VkInternalAllocationType allocationType,
                                  VkSystemAllocationScope allocationScope) {

    }

    std::shared_ptr<vk::AllocationCallbacks> acquireAllocCb() {
        if(!instance) {
            instance = std::make_shared<vk::AllocationCallbacks>();
            instance->pfnAllocation = allocationFunction;
            instance->pfnFree = freeFunction;
            instance->pfnReallocation = reallocationFunction;
            instance->pfnInternalAllocation = internalAllocationNotification;
            instance->pfnInternalFree = internalFreeNotification;
        }

        return instance;
    }
}
