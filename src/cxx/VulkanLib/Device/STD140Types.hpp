//
// Created by kgaft on 3/13/24.
//

#ifndef VULKANLIB_STD140TYPES_HPP
#define VULKANLIB_STD140TYPES_HPP

#include <cstdint>

struct UIN64_T_STD140{
    alignas(4) uint32_t firstVal = 0;
    alignas(4) uint32_t secondVal = 0;
    UIN64_T_STD140(uint64_t value){
        if(value>=UINT32_MAX){
            firstVal = UINT32_MAX;
            secondVal = value-UINT32_MAX;
        } else {
            firstVal = value;
        }
    }
};
#endif //MENGINE_STD140TYPES_HPP
