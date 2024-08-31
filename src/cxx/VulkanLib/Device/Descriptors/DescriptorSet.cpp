//
// Created by kgaft on 3/11/24.
//
#include "DescriptorSet.hpp"

void DescriptorSet::updateDescriptors() {

    uint32_t counter = 0;
    for (auto &item: descriptorSet) {

        for (auto &cItem: writes) {
            cItem.dstSet = item;
        }
        if (imageInfoPerInstanceAmount == 0 && bufferInfoPerInstanceAmount == 0) {
            device->getDevice().updateDescriptorSets((uint32_t)writes.size(), writes.data(), 0, nullptr);
        } else {
            device->getDevice().updateDescriptorSets(imageInfoPerInstanceAmount + bufferInfoPerInstanceAmount,
                                                     &writes[counter], 0, nullptr);
        }
        counter += imageInfoPerInstanceAmount + bufferInfoPerInstanceAmount;
    }
}

void DescriptorSet::clearObjectsInfos() {
    for (auto &item: imagesInfo) {
        delete item;
    }
    imagesInfo.clear();
    for (const auto &item: buffersInfo) {
        delete item;
    }
    buffersInfo.clear();
    asInfo.clear();
    writes.clear();
}

void DescriptorSet::bindDescriptor(vk::PipelineBindPoint bindPoint, uint32_t currentInstance, vk::CommandBuffer cmd,
                                   vk::PipelineLayout pipelineLayout) {
    cmd.bindDescriptorSets(bindPoint, pipelineLayout, 0, 1, &descriptorSet[currentInstance], 0, nullptr);
}

void DescriptorSet::addBufferInfo(DescriptorBufferInfo &bufferInfo) {
    buffersInfo.push_back(new DescriptorBufferInfo(bufferInfo));
    writes.push_back({});
    writes[writes.size() - 1].sType = vk::StructureType::eWriteDescriptorSet;
    writes[writes.size() - 1].dstSet = descriptorSet[0];
    writes[writes.size() - 1].dstBinding = buffersInfo[buffersInfo.size() - 1]->binding;
    writes[writes.size() - 1].dstArrayElement = 0;
    writes[writes.size() - 1].descriptorType = buffersInfo[buffersInfo.size() - 1]->descriptorType;
    writes[writes.size() - 1].descriptorCount =(uint32_t) buffersInfo[buffersInfo.size() - 1]->base.size();
    writes[writes.size() - 1].pBufferInfo = buffersInfo[buffersInfo.size() - 1]->base.data();
}

void DescriptorSet::addImageInfo(DescriptorImageInfo &imageInfo) {
    imagesInfo.push_back(new DescriptorImageInfo(imageInfo));
    writes.push_back({});
    writes[writes.size() - 1].sType = vk::StructureType::eWriteDescriptorSet;
    writes[writes.size() - 1].dstSet = descriptorSet[0];
    writes[writes.size() - 1].dstBinding = imagesInfo[imagesInfo.size() - 1]->binding;
    writes[writes.size() - 1].dstArrayElement = 0;
    writes[writes.size() - 1].descriptorType = imagesInfo[imagesInfo.size() - 1]->descriptorType;
    writes[writes.size() - 1].descriptorCount = (uint32_t)imagesInfo[imagesInfo.size() - 1]->imageInfos.size();
    writes[writes.size() - 1].pImageInfo = imagesInfo[imagesInfo.size() - 1]->imageInfos.data();
}

void DescriptorSet::addAccelerationStructureInfo(DescriptorAccelerationStructureInfo &acsInfo) {
    asInfo.push_back(new DescriptorAccelerationStructureInfo(acsInfo));
    writes.push_back({});

    writes[writes.size() - 1].sType = vk::StructureType::eWriteDescriptorSet;
    writes[writes.size() - 1].dstSet = descriptorSet[0];
    writes[writes.size() - 1].dstBinding = asInfo[asInfo.size() - 1]->binding;
    writes[writes.size() - 1].dstArrayElement = 0;
    writes[writes.size() - 1].descriptorType = asInfo[asInfo.size() - 1]->descriptorType;
    writes[writes.size() - 1].descriptorCount = 1;
    writes[writes.size() - 1].pNext = &asInfo[asInfo.size() - 1]->base;
}

void DescriptorSet::setImageInfoPerInstanceAmount(uint32_t imageInfoPerInstanceAmount) {
    DescriptorSet::imageInfoPerInstanceAmount = imageInfoPerInstanceAmount;
}

void DescriptorSet::setBufferInfoPerInstanceAmount(uint32_t bufferInfoPerInstanceAmount) {
    DescriptorSet::bufferInfoPerInstanceAmount = bufferInfoPerInstanceAmount;
}
