//
// Created by kgaft on 11/4/23.
//
#pragma once

#include <vulkan/vulkan.hpp>
#include "InstanceBuilder.hpp"

class Instance {
public:
    Instance(InstanceBuilder *pBuilder) {
        vk::ApplicationInfo appInfo(
                pBuilder->applicationName,
                VK_MAKE_VERSION(1, 0, 0),
                "VulkanLib",
                VK_API_VERSION_1_3
        );
        vk::InstanceCreateInfo createInfo(
                vk::InstanceCreateFlags(),
                &appInfo,
                pBuilder->layers.size(), pBuilder->layers.data(),
                pBuilder->extensions.size(), pBuilder->extensions.data()
        );
        try {
            instance = vk::createInstance(createInfo, nullptr);
        } catch (vk::SystemError &error) {
            std::cerr << error.what() << std::endl;
        }
    }

private:
    vk::Instance instance{nullptr};
};




