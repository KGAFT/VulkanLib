//
// Created by kgaft on 4/1/2024.
//

#ifndef MENGINE_PRESENTPASSTHROUGH_H
#define MENGINE_PRESENTPASSTHROUGH_H


#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "VulkanLib/Device/Image/Image.hpp"

class PresentPassThrough {
public:
    virtual void initializeDevices(std::shared_ptr<LogicalDevice> source, std::shared_ptr<LogicalDevice> target) = 0;
    virtual void processCopy() = 0;
    virtual std::shared_ptr<Image> getRenderTargetForSourceDevice() = 0;
    virtual void presentImage() = 0;
};


#endif //MENGINE_PRESENTPASSTHROUGH_H
