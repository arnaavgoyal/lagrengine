#ifndef GRAPHICS_TEXTURE_H
#define GRAPHICS_TEXTURE_H

#include <string>

/**
 * Represents an OpenGL texture
 */
struct Texture {
    /** the id of the texture */
    unsigned int id;

    /**
     * Creates a texture from the specified file path
     * @path the path to the file
     * @return whether or not the texture was successfully created
     */
    bool create(std::string path);

    /**
     * Destroys the given texture
     */
    void destroy();
};

#endif // GRAPHICS_TEXTURE_H
