//
// Created by kgaft on 11/5/23.
//
#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

class DeviceBuilder{
    friend class DeviceSuitability;
    friend class LogicalDevice;
private:
    std::vector<const char*> requestExtensions;
    bool requirePresentSupport = false;
    bool requireGraphicsSupport = false;
    vk::SurfaceKHR surfaceForPresentationCheck;

public:
    void addExtension(const char* extensionName){
        requestExtensions.push_back(extensionName);
    }
    void addExtensions(const char** pExtensions, uint32_t extensionCount){
        for (int i = 0; i < extensionCount; ++i){
            requestExtensions.push_back(pExtensions[i]);
        }
    }
    void requestGraphicsSupport(){
        requestExtensions.push_back("VK_KHR_dynamic_rendering");
        requestExtensions.push_back("VK_KHR_depth_stencil_resolve");
        requestExtensions.push_back("VK_KHR_create_renderpass2");
        requestExtensions.push_back("VK_KHR_multiview");
        requestExtensions.push_back("VK_KHR_maintenance2");
        requireGraphicsSupport = true;
    }
    void requestPresentSupport(VkSurfaceKHR surface){
        surfaceForPresentationCheck = vk::SurfaceKHR(surface);
        requirePresentSupport = true;
        requestExtensions.push_back("VK_KHR_swapchain");
    }
};