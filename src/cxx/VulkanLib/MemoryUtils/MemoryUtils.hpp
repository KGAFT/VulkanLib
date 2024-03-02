//
// Created by kgaft on 11/5/23.
//
#pragma once

#include <cstddef>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace Integral {
    template<class Integral>
    constexpr bool isAligned(Integral x, size_t a) noexcept {
        return (x & (Integral(a) - 1)) == 0;
    }

    template<class Integral>
    constexpr Integral alignUp(Integral x, size_t a) noexcept {
        return Integral((x + (Integral(a) - 1)) & ~Integral(a - 1));
    }

    template<class Integral>
    constexpr Integral alignDown(Integral x, size_t a) noexcept {
        return Integral(x & ~Integral(a - 1));
    }


}

class MemoryUtils{
public:
    static void memClear(void *data, size_t size) {
        for (size_t i = 0; i < size; ++i) {
            ((char *) data)[i] = 0;
        }
    }

    static VkTransformMatrixKHR glmMat4toTransformMatrixKHR(glm::mat4 matrix) {
        // VkTransformMatrixKHR uses a row-major memory layout, while glm::mat4
        // uses a column-major memory layout. We transpose the matrix so we can
        // memcpy the matrix's data directly.
        auto temp = glm::mat3x4(glm::transpose(matrix));
        VkTransformMatrixKHR out_matrix;
        memcpy(&out_matrix, &temp, sizeof(glm::mat3x4));
        return out_matrix;
    }
};