//
// Created by kgaft on 3/11/24.
//
#include "ImageView.hpp"

ImageView::ImageView(vk::ImageCreateInfo &parentInfo, std::shared_ptr<LogicalDevice> device, vk::ImageView base,
                     vk::ImageViewCreateInfo &createInfo) : parentInfo(parentInfo), device(device),  base(base),
                                                            createInfo(createInfo)  {}

const vk::ImageView &ImageView::getBase() const {
    return base;
}

const vk::ImageViewCreateInfo &ImageView::getCreateInfo() const {
    return createInfo;
}

vk::ImageCreateInfo &ImageView::getParentInfo() const {
    return parentInfo;
}

void ImageView::destroy() {
    device->getDevice().destroyImageView(base);
    destroyed = true;
}
