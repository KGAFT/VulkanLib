//
// Created by kgaft on 11/9/23.
//
#pragma once

#include <shaderc/shaderc.hpp>
#include <fstream>
#include <iostream>
#include <VulkanLib/MemoryUtils/SerialObject.hpp>
#include <cstring>

struct ShaderCompileResult {
    const char *binary;
    size_t shaderSize;
};

class ShaderLoader {
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
        compiler = shaderc_compiler_initialize();
        shaderc_compile_options_set_include_callbacks(nullptr, GetInclude,
                                                      ReleaseInclude, nullptr);
    }

private:
    shaderc_compiler_t compiler;
private:
    static SerialObject<shaderc_include_result> includes;
    static shaderc_include_result *GetInclude(void* user_data, const char* requested_source, int type,
                                              const char* requesting_source, size_t include_depth) {
        auto *result = (shaderc_include_result *) includes.getObjectInstance();
        size_t codeLength;
        const char* code = readCode(requested_source, &codeLength);
        if(!code){
            throw std::runtime_error(std::string("Include error: no such file: ")+requested_source+" included in "+requesting_source);
        }
        result->content = code;
        result->content_length = codeLength;
        result->source_name = requested_source;
        result->source_name_length = strlen(requested_source);
        return result;
    }

    static void ReleaseInclude(void* userData, shaderc_include_result* data){
        delete data->content;
        includes.releaseObjectInstance(data);
    }

    static const char *readCode(const char *filePath, size_t* sizeOutput) {
        std::ifstream fileReader(filePath, std::ios::binary);
        if (fileReader) {
            char *content;
            fileReader.seekg(0, std::ios::end);
            size_t size = fileReader.tellg();
            content = new char[size];
            fileReader.seekg(0, std::ios::beg);
            fileReader.read(&content[0], size);
            fileReader.close();
            return content;
        }
        return nullptr;
    }

};

