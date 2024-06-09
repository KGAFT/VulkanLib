//
// Created by kgaft on 11/10/23.
//
#pragma once

#include <vulkan/vulkan.hpp>

enum ShaderFileType{
    BINARY_FILE,
    SRC_FILE
};

struct AdditionalLine {
    std::string data;
    uint32_t lineIndex = 0;
};

struct ShaderCreateInfo{
    const char* pathToFile;
    const char* fileName;
    ShaderFileType fileType;
    vk::ShaderStageFlagBits stage;
    std::vector<AdditionalLine> additionalLines;
};
