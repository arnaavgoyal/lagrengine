#ifndef GRAPHICS_UNIFORM_H
#define GRAPHICS_UNIFORM_H

#include <string>

template<typename T>
struct Uniform {
    std::string name;
    T value;
};

using TextureUniform = Uniform<int>;

#endif // GRAPHICS_UNIFORM_H
