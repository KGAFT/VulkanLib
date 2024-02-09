//
// Created by kgaft on 11/10/23.
//
#pragma once

#include <shaderc/shaderc.hpp>
#include <VulkanLib/MemoryUtils/SeriesObject.hpp>
#include <cstring>
#include <VulkanLib/MemoryUtils/FileReader.hpp>
class ShaderLoaderIncluder : public shaderc::CompileOptions::IncluderInterface{
private:
    SeriesObject<shaderc_include_result> includes;

public:
    shaderc_include_result *
    GetInclude(const char *requested_source, shaderc_include_type type, const char *requesting_source,
               size_t include_depth) override {
        if(type==shaderc_include_type_relative){
            std::cout<<"Include relative"<<std::endl;
        }
        if(include_depth>1){
            std::cout<<include_depth<<std::endl;
        }
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
        try{
            delete data->content;
        }catch(std::exception& e){

        }
        includes.releaseObjectInstance(data);
    }
};

