//
// Created by kgaft on 11/7/23.
//
#pragma once

#include <vector>

namespace VectorUtils{
    template <typename T> void removeRepeatingElements(std::vector<T>& data){
        size_t size = data.size();
        for (size_t i = 0; i < size; i++){
            for(size_t ii = i+1; ii<size;ii++){
                if(data[i]==data[ii]){
                    data.erase(data.begin()+ii);
                    ii--;
                    size--;
                }
            }
        }
    }
}