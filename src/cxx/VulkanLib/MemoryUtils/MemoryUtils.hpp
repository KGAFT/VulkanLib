//
// Created by kgaft on 11/5/23.
//
#pragma once

#include <cstddef>

namespace MemoryUtils{
    template <class Integral>
    constexpr bool isAligned(Integral x, size_t a) noexcept
    {
        return (x & (Integral(a) - 1)) == 0;
    }

    template <class Integral>
    constexpr Integral alignUp(Integral x, size_t a) noexcept
    {
        return Integral((x + (Integral(a) - 1)) & ~Integral(a - 1));
    }

    template <class Integral>
    constexpr Integral alignDown(Integral x, size_t a) noexcept
    {
        return Integral(x & ~Integral(a - 1));
    }
    void memClear(void* data, size_t size){
        for (size_t i = 0; i < size; ++i){
            ((char*)data)[i] = 0;
        }
    }
}