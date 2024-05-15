#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "graphics/model.h"

#include "utils/obj_loader.h"

bool Model::create(std::string path) {
    return obj_loader::loadObj(meshes, path);
}

void Model::destroy() {
    for(Mesh m : meshes) {
        m.destroy();
    }
}
