//
// Created by kgaft on 11/5/23.
//
#pragma once

#include <cstddef>

namespace MemoryUtils{
    void memClear(void* data, size_t size){
        for (size_t i = 0; i < size; ++i){
            ((char*)data)[i] = 0;
        }
    }
}