//
// Created by kgaft on 11/9/23.
//
#pragma once

#include <shaderc/shaderc.hpp>
#include <fstream>
#include <iostream>
#include <VulkanLib/MemoryUtils/SerialObject.hpp>
#include <cstring>
#include <vulkan/vulkan.hpp>
#include "ShaderLoaderIncluder.hpp"
#include "VulkanLib/MemoryUtils/FileReader.hpp"
#include "ShaderCreateInfo.hpp"

struct ShaderCompileResult {
    const char *binary;
    size_t shaderSize;
};


class ShaderLoader{
private:
    static inline ShaderLoader *instance = nullptr;

public:
    static const ShaderLoader *getInstance() {
        if (!instance) {
            instance = new ShaderLoader;
        }
        return instance;
    }

private:
    ShaderLoader() {
        compileOptions.SetIncluder(std::unique_ptr<shaderc::CompileOptions::IncluderInterface>{std::make_unique<ShaderLoaderIncluder>(ShaderLoaderIncluder())});

    }

private:
    shaderc::Compiler compiler;
    vk::ShaderModuleCreateInfo shaderCreateInfo;
    shaderc::CompileOptions compileOptions;
    std::vector<uint32_t> shaderBuffer;
    const char* shaderBinaryBuffer;
    size_t binarySize;
public:
    vk::ShaderModule createShaderModule(LogicalDevice& device, ShaderCreateInfo& createInfo){
        if(createInfo.fileType == BINARY_FILE){
            FileReader::readBinary(createInfo.pathToFile, &binarySize);
        } else{
            compileShader(createInfo.pathToFile, createInfo.fileName, vkTypeToShadercType(createInfo.stage), shaderBuffer);
        }
        MemoryUtils::memClear(&shaderCreateInfo, sizeof(vk::ShaderModuleCreateInfo));
        shaderCreateInfo.sType = vk::StructureType::eShaderCreateInfoEXT;
        shaderCreateInfo.pCode = createInfo.fileType==SRC_FILE?shaderBuffer.data():reinterpret_cast<const uint32_t*>(shaderBinaryBuffer);
        shaderCreateInfo.codeSize = createInfo.fileType==SRC_FILE?shaderBuffer.size():binarySize;
        vk::ShaderModule result = device.getDevice().createShaderModule(shaderCreateInfo);
        if(createInfo.fileType == BINARY_FILE){
            free((void *) shaderBinaryBuffer);
        } else{
            shaderBuffer.clear();
        }
    }
    void compileShader(const char* filePath, const char* fileName, shaderc_shader_kind shaderKind, std::vector<uint32_t>& output) const{
        size_t codeSize;
        const char* shaderCode = FileReader::readText(filePath, &codeSize);

        shaderc::CompilationResult<uint32_t> result = compiler.CompileGlslToSpv(shaderCode,
                                                                       codeSize * sizeof(char),
                                                                       shaderKind, fileName, compileOptions);
        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            throw std::runtime_error("Failed to compile shader " + std::string(filePath) + " into SPIR-V:\n " +
                                             result.GetErrorMessage());
        }
        free((void *) shaderCode);
        output = std::vector<uint32_t>(result.cbegin(), result.cend());
    }
public:
    static shaderc_shader_kind vkTypeToShadercType(vk::ShaderStageFlagBits shaderType){
        switch (shaderType) {
            case vk::ShaderStageFlagBits::eCompute:
                return shaderc_compute_shader;
            case vk::ShaderStageFlagBits::eFragment:
                return shaderc_fragment_shader;
            case vk::ShaderStageFlagBits::eVertex:
                return shaderc_vertex_shader;
            case vk::ShaderStageFlagBits::eGeometry:
                return shaderc_geometry_shader;
            case vk::ShaderStageFlagBits::eMeshEXT:
                return shaderc_mesh_shader;
        }
    }


};

