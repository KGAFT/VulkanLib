//
// Created by kgaft on 11/10/23.
//
#pragma once

#include <shaderc/shaderc.hpp>
#include <VulkanLib/MemoryUtils/SeriesObject.hpp>
#include <cstring>
#include <VulkanLib/MemoryUtils/FileReader.hpp>
#include <stdexcept>
class ShaderLoaderIncluder : public shaderc::CompileOptions::IncluderInterface{
public:
    ShaderLoaderIncluder(std::string &workDirectory, const std::vector<std::string> &includeDirectories)
            : workDirectory(workDirectory), includeDirectories(includeDirectories) {}

private:
    SeriesObject<shaderc_include_result> includes;
    std::string workDirectory;
    const std::vector<std::string> includeDirectories;
public:
    shaderc_include_result *
    GetInclude(const char *requested_source, shaderc_include_type type, const char *requesting_source,
               size_t include_depth) override {

        auto *result = (shaderc_include_result *) includes.getObjectInstance();
        size_t codeLength;
        std::vector<AdditionalLine> additionalLine;
        const char* code = nullptr; 
        try{
            code = FileReader::readText((workDirectory+std::string(requested_source)).c_str(), &codeLength, additionalLine);;
        }catch(std::runtime_error& e){

        }
        
        if(!code){
            for (auto &item: includeDirectories){
                try{
                code = FileReader::readText((item+"/"+std::string(requested_source)).c_str(), &codeLength, additionalLine);
                }catch(std::runtime_error& e){
                    
                }
                if(code){
                    break;
                }
            }
        }
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

