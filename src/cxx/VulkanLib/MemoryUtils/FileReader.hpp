//
// Created by kgaft on 11/10/23.
//
#pragma once

#include <fstream>

namespace FileReader{
    const char *readText(const char *filePath, size_t* sizeOutput) {
        std::ifstream fileReader(filePath, std::ios::binary);
        if (fileReader) {
            char *content;
            fileReader.seekg(0, std::ios::end);
            size_t size = fileReader.tellg();
            content = new char[size];
            fileReader.seekg(0, std::ios::beg);
            fileReader.read(&content[0], size);
            fileReader.close();
            *sizeOutput = size;
            return content;
        }
        return nullptr;
    }
    const char* readBinary(const char* filePath, size_t* sizeOutput){
        std::ifstream file{filePath, std::ios::ate | std::ios::binary};

        if (!file.is_open())
        {
            throw std::runtime_error(std::string("Failed to open file: ") + filePath);
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        char* buffer = new char[fileSize];

        file.seekg(0);
        file.read(buffer, fileSize);
        *sizeOutput = fileSize;
        file.close();
        return buffer;
    }
}