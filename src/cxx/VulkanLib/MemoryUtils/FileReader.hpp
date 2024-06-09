//
// Created by kgaft on 11/10/23.
//
#pragma once

#include <fstream>

#include "VulkanLib/Shader/ShaderCreateInfo.hpp"

class FileReader {
public:
    static const char *readText(const char *filePath, size_t *sizeOutput,
                                std::vector<AdditionalLine> &additionalLines) {
        std::ifstream fileReader(filePath, std::ios::binary);
        if (fileReader) {
            if (additionalLines.empty()) {
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
            uint32_t i = 0;
            std::string currentLine;
            std::ostringstream oss;
            while (getline(fileReader, currentLine)) {
                for (auto &additionalLine: additionalLines) {
                    if (additionalLine.lineIndex == i) {
                        oss << additionalLine.data << std::endl;
                        break;
                    }
                }
                oss << currentLine << std::endl;
                i++;
            }
            auto str = oss.str();
            char* content = new char[str.size()];
            memcpy(content, str.c_str(), str.size());
            *sizeOutput = str.size();
            fileReader.close();
            return content;
        }
        throw std::runtime_error("Failed to open file");
    }

    static const char *readBinary(const char *filePath, size_t *sizeOutput) {
        std::ifstream file{filePath, std::ios::ate | std::ios::binary};

        if (!file.is_open()) {
            throw std::runtime_error(std::string("Failed to open file: ") + filePath);
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        char *buffer = new char[fileSize];

        file.seekg(0);
        file.read(buffer, fileSize);
        *sizeOutput = fileSize;
        file.close();
        return buffer;
    }
};
