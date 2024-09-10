//
// Created by kgaft on 11/5/23.
//
#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

class DeviceBuilder {
    friend class DeviceSuitability;

    friend class LogicalDevice;

private:
    std::vector<const char *> requestExtensions;
    bool requirePresentSupport = false;
    bool requireGraphicsSupport = false;
    bool requireComputeSupport = false;
    vk::SurfaceKHR surfaceForPresentationCheck;
    bool rayTracingSupport = false;
public:
    void addExtension(const char *extensionName) {
        requestExtensions.push_back(extensionName);
    }

    void addExtensions(const char **pExtensions, uint32_t extensionCount) {
        for (uint32_t i = 0; i < extensionCount; ++i) {
            requestExtensions.push_back(pExtensions[i]);
        }
    }

    void requestGraphicsSupport() {
        requestExtensions.push_back("VK_KHR_dynamic_rendering");
        requestExtensions.push_back("VK_KHR_depth_stencil_resolve");
        requestExtensions.push_back("VK_KHR_create_renderpass2");
        requestExtensions.push_back("VK_KHR_multiview");
        requestExtensions.push_back("VK_KHR_maintenance2");
        requireGraphicsSupport = true;
    }

    void requestRayTracingSupport() {
      requestExtensions.push_back("VK_KHR_acceleration_structure");
      requestExtensions.push_back("VK_KHR_ray_tracing_pipeline");
      requestExtensions.push_back("VK_KHR_ray_query");
      requestExtensions.push_back("VK_KHR_pipeline_library");
      requestExtensions.push_back("VK_KHR_deferred_host_operations");
      requestExtensions.push_back("VK_KHR_spirv_1_4");
      requestExtensions.push_back("VK_EXT_descriptor_indexing");
      requestExtensions.push_back("VK_KHR_buffer_device_address");
      requestExtensions.push_back("VK_KHR_shader_float_controls");
      requestExtensions.push_back("VK_KHR_maintenance3");
      requestExtensions.push_back("VK_KHR_device_group");
      rayTracingSupport = true;
    }

    [[nodiscard]] bool requirePresentSupport1() const {
      return requirePresentSupport;
    }
    [[nodiscard]] bool requireGraphicsSupport1() const {
      return requireGraphicsSupport;
    }
    [[nodiscard]] bool requireComputeSupport1() const {
      return requireComputeSupport;
    }
    [[nodiscard]] bool rayTracingSupport1() const { return rayTracingSupport; }

    void requestComputeSupport() {
        requireComputeSupport = true;
    }

    void requestPresentSupport(VkSurfaceKHR surface) {
        surfaceForPresentationCheck = vk::SurfaceKHR(surface);
        requirePresentSupport = true;
        requestExtensions.push_back("VK_KHR_swapchain");
    }
};