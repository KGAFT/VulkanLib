//
// Created by kgaft on 11/10/23.
//
#pragma once

#include <shaderc/shaderc.hpp>
#include <VulkanLib/MemoryUtils/SerialObject.hpp>
#include <cstring>
#include <VulkanLib/MemoryUtils/FileReader.hpp>
class ShaderLoaderIncluder : public shaderc::CompileOptions::IncluderInterface{
private:
    SerialObject<shaderc_include_result> includes;

public:
    shaderc_include_result *
    GetInclude(const char *requested_source, shaderc_include_type type, const char *requesting_source,
               size_t include_depth) override {
        auto *result = (shaderc_include_result *) includes.getObjectInstance();
        size_t codeLength;
        const char* code = FileReader::readText(requested_source, &codeLength);
        if(!code){
            throw std::runtime_error(std::string("Include error: no such file: ")+requested_source+" included in "+requesting_source);
        }
        result->content = code;
        result->content_length = codeLength;
        result->source_name = requested_source;
        result->source_name_length = strlen(requested_source);
        return result;
    }


    void ReleaseInclude(shaderc_include_result* data) override{
        free((void *) data->content);
        includes.releaseObjectInstance(data);
    }
};

