//
// Created by kgaft on 11/4/23.
//
#ifndef VULKANLIB_INSTANCE_HPP
#define VULKANLIB_INSTANCE_HPP
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include "InstanceBuilder.hpp"
#include "VulkanLib/InstanceLogger/InstanceLogger.hpp"
#include "VulkanLib/MemoryUtils/IDestroyableObject.hpp"

class Instance : IDestroyableObject {
public:
    static bool debugSupported();

public:
    explicit Instance(InstanceBuilder &pBuilder);

    Instance() = default;

private:
    vk::Instance instance{nullptr};
    vk::detail::DispatchLoaderDynamic dynamicLoader;
    std::vector<const char *> enabledLayers;
    InstanceLogger *logger = nullptr;
public:
    [[nodiscard]] InstanceLogger *getLogger() const;

    vk::Instance &getInstance();

    [[nodiscard]] const std::vector<const char *> &getEnabledLayers() const;

    vk::detail::DispatchLoaderDynamic &getDynamicLoader();

public:
    void destroy() override;

};

#endif



