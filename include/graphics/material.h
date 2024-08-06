#ifndef GRAPHICS_MATERIAL_H
#define GRAPHICS_MATERIAL_H

#include <string>

#include "texture.h"

struct Material {
    std::string name;
    Texture ambient;
    Texture diffuse;
    Texture specular;
    float shininess;
};

#endif // GRAPHICS_MATERIAL_H
