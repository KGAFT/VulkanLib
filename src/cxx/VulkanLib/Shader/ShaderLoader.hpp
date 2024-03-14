//
// Created by kgaft on 11/9/23.
//
#pragma once

#include <shaderc/shaderc.hpp>
#include <fstream>
#include <iostream>
#include <VulkanLib/MemoryUtils/SeriesObject.hpp>
#include <cstring>
#include <vulkan/vulkan.hpp>
#include "ShaderLoaderIncluder.hpp"
#include "VulkanLib/MemoryUtils/FileReader.hpp"
#include "ShaderCreateInfo.hpp"
#include "VulkanLib/Pipelines/Shader.hpp"


class ShaderLoader {
private:
    static inline ShaderLoader *instance = nullptr;

public:
    static ShaderLoader *getInstance() {
        if (!instance) {
            instance = new ShaderLoader;
        }
        return instance;
    }

private:
    ShaderLoader() {

    }

private:
    shaderc::Compiler compiler;

    std::unique_ptr<ShaderLoaderIncluder> include;
    std::vector<std::string> includeDirectories;
    vk::ShaderModuleCreateInfo shaderCreateInfo;
    std::vector<uint32_t> shaderBuffer;
    const char *shaderBinaryBuffer;
    size_t binarySize;
public:
    Shader *createShader(LogicalDevice &device, std::vector<ShaderCreateInfo> &createInfos) {
        std::vector<vk::PipelineShaderStageCreateInfo> infos;
        unsigned int counter = 0;
        infos.resize(createInfos.size());
        for (auto &item: createInfos) {
            vk::ShaderModule module = createShaderModule(device, item);
            infos[counter].stage = item.stage;
            infos[counter].module = module;
            infos[counter].pName = "main";
            counter++;
        }
        return new Shader(infos, device);
    }

    vk::ShaderModule createShaderModule(LogicalDevice &device, ShaderCreateInfo &createInfo) {
        if (createInfo.fileType == BINARY_FILE) {
            FileReader::readBinary(createInfo.pathToFile, &binarySize);
        } else {
            compileShader(createInfo.pathToFile, createInfo.fileName, vkTypeToShadercType(createInfo.stage),
                          shaderBuffer);
        }
        MemoryUtils::memClear(&shaderCreateInfo, sizeof(vk::ShaderModuleCreateInfo));
        shaderCreateInfo.sType = vk::StructureType::eShaderModuleCreateInfo;
        shaderCreateInfo.pCode = createInfo.fileType == SRC_FILE ? shaderBuffer.data()
                                                                 : reinterpret_cast<const uint32_t *>(shaderBinaryBuffer);
        shaderCreateInfo.codeSize =
                createInfo.fileType == SRC_FILE ? shaderBuffer.size() * sizeof(uint32_t) : binarySize;
        vk::ShaderModule result = device.getDevice().createShaderModule(shaderCreateInfo);
        if (createInfo.fileType == BINARY_FILE) {
            try {
                delete shaderBinaryBuffer;
            } catch (std::exception &e) {

            }
        } else {
            shaderBuffer.clear();
        }
        return result;
    }

    void compileShader(const char *filePath, const char *fileName, shaderc_shader_kind shaderKind,
                       std::vector<uint32_t> &output) const {
        size_t codeSize;
        const char *shaderCode = FileReader::readText(filePath, &codeSize);
        std::string workDirectory(filePath);
        workDirectory = workDirectory.substr(0, workDirectory.size() - strlen(fileName));
        shaderc::CompileOptions compileOptions;
        compileOptions.SetTargetSpirv(shaderc_spirv_version_1_5);


        compileOptions.SetIncluder(std::unique_ptr<shaderc::CompileOptions::IncluderInterface>{
                std::make_unique<ShaderLoaderIncluder>(ShaderLoaderIncluder(workDirectory, includeDirectories))});

        shaderc::CompilationResult<uint32_t> result = compiler.CompileGlslToSpv(shaderCode,
                                                                                codeSize * sizeof(char),
                                                                                shaderKind, fileName, compileOptions);
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            throw std::runtime_error("Failed to compile shader " + std::string(filePath) + " into SPIR-V:\n " +
                                     result.GetErrorMessage());
        }
        try {
            delete shaderCode;

        } catch (std::exception &e) {

        }

        output = std::vector<uint32_t>(result.cbegin(), result.cend());
    }

    std::vector<std::string> &getIncludeDirectories() {
        return includeDirectories;
    }

public:
    static shaderc_shader_kind vkTypeToShadercType(vk::ShaderStageFlagBits shaderType) {
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
            case vk::ShaderStageFlagBits::eRaygenKHR:
                return shaderc_raygen_shader;
            case vk::ShaderStageFlagBits::eClosestHitKHR:
                return shaderc_closesthit_shader;
            case vk::ShaderStageFlagBits::eAnyHitKHR:
                return shaderc_anyhit_shader;
            case vk::ShaderStageFlagBits::eMissKHR:
                return shaderc_miss_shader;
            case vk::ShaderStageFlagBits::eIntersectionKHR:
                return shaderc_intersection_shader;
            default:
                return shaderc_vertex_shader;
        }
    }


};

