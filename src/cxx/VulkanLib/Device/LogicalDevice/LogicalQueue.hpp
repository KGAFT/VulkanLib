//
// Created by kgaft on 11/5/23.
//

#pragma once

#include <vulkan/vulkan.hpp>

class LogicalQueue {
    friend class LogicalDevice;
public:
    LogicalQueue(vk::Queue queue, bool supportPresentation, vk::QueueFlagBits queueType) : queue(queue),
                                                                                                  supportPresentation(
                                                                                                          supportPresentation),
                                                                                                  queueType(
                                                                                                          queueType) {
    }

private:
    vk::Queue queue;
    bool supportPresentation;
    vk::QueueFlagBits queueType;
public:
    const vk::Queue &getQueue() const {
        return queue;
    }

    bool isSupportPresentation() const {
        return supportPresentation;
    }

    vk::QueueFlagBits getQueueType() const {
        return queueType;
    }
};


