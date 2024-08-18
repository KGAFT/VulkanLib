//
// Created by kgaft on 8/18/24.
//

#ifndef IRESIZECALLBACK_HPP
#define IRESIZECALLBACK_HPP
#include <cstdint>

class IResizeCallback {
public:
    virtual void resized(uint32_t width, uint32_t height) = 0;
};
#endif //IRESIZECALLBACK_H
