#ifndef UTILS_OBJ_LOADER_H
#define UTILS_OBJ_LOADER_H

namespace obj_loader {
/**
 * Loads an OBJ file into a vector of meshes
 * @param meshes the destination to load to
 * @param path the path to the OBJ file
 * @return whether or not the OBJ file was successfully read
 */
bool loadObj(std::vector<Mesh> &meshes, std::string path);

/**
 * Loads a MTL file into a vector of materials
 * @param materials the destination to load to
 * @param path the path to the MTL file
 * @return whether or not the MTL file was successfully read
 */
bool loadMtl(std::vector<Material> &materials, std::string path);
};

#endif // UTILS_OBJ_LOADER_H
