#ifndef GRAPHICS_TEXTURE_H
#define GRAPHICS_TEXTURE_H

#include <string>

/**
 * Represents an OpenGL texture
 */
struct Texture {
    /** the id of the texture */
    unsigned int id;

    bool create(std::string path);
    void destroy();
};

#endif // GRAPHICS_TEXTURE_H
