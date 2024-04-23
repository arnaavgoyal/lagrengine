#ifndef GRAPHICS_TEXTURE_H
#define GRAPHICS_TEXTURE_H

#include <string>

struct Texture {
    unsigned int id;

    bool create(std::string path);
    void destroy();
};

#endif // GRAPHICS_TEXTURE_H
