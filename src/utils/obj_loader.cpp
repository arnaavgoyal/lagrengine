#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "graphics/mesh.h"
#include "graphics/model.h"
#include "graphics/vertex.h"

#include "utils/obj_loader.h"

namespace obj_loader {

static void parseLineFloats(std::vector<float> &dest, std::string line) {
    int start = 0;
    for(start = 0; line[start] != ' ' && line[start] != '\n' &&
            line[start] != '\0'; start++);

    while(line[start] != '\0' && line[start] != '\n') {
        int end;
        start++;
        for(end = start; line[end] != ' ' && line[end] != '\n' &&
                line[end] != '\0'; end++);
        dest.push_back(std::stof(line.substr(start, end - start)));
        start = end;
    }
}

static void parseLineStrings(std::vector<std::string> &dest, std::string line) {
    int start = 0;
    for(start = 0; line[start] != ' ' && line[start] != '\n' &&
            line[start] != '\0'; start++);

    while(line[start] != '\0' && line[start] != '\n') {
        int end;
        start++;
        for(end = start; line[end] != ' ' && line[end] != '\n' &&
                line[end] != '\0'; end++);
        dest.push_back(line.substr(start, end - start));
        start = end;
    }
}

static void parseVertexString(std::vector<int> &values,
        std::string string_vertex) {
    int start = -1;
    
    do {
        int end;
        start++;

        for(end = start; string_vertex[end] != '/'
                && string_vertex[end] != '\0'; end++);

        std::string string_value = string_vertex.substr(start, end - start);
        int value;

        if(string_value == "") {
            value = 0;
        }

        else {
            value = std::stoi(string_value);
        }

        values.push_back(value);
        start = end;
    }

    while(string_vertex[start] != '\0');
}

static void createFace(std::vector<Vertex> &face,
        std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals,
        std::vector<glm::vec2> &uvs, std::string line) {
    std::vector<std::string> string_vertices;
    parseLineStrings(string_vertices, line);

    for(int i = 0; i < string_vertices.size(); i++) {
        std::vector<int> values;
        parseVertexString(values, string_vertices[i]);
        Vertex v;
        v.position = positions[values[0] - 1];
        v.uv = uvs[values[1] - 1];
        v.normal = normals[values[2] - 1];
        face.push_back(v);
    }
}

static glm::vec3 createFaceNormal(std::vector<Vertex> &face) {
    glm::vec3 normal = glm::vec3(0, 0, 0);
    for(int i = 0; i < face.size(); i++) {
        if(i == face.size() - 1) {
            normal += glm::cross(face[i].position,
                    face[0].position);
        }
        else {
            normal += glm::cross(face[i].position,
                    face[i + 1].position);
        }
    }

    return glm::normalize(normal);
}

static bool isEar(std::vector<Vertex> &face, glm::vec3 normal, int vi) {
    int face_size = face.size();

    if(vi >= face_size || vi < 0)
        return false;

    int pvi = (vi == 0) ? (face_size - 1) : (vi - 1);
    int nvi = (vi == face_size - 1) ? (0) : (vi + 1);

    glm::vec3 v = face[vi].position;
    glm::vec3 pv = face[pvi].position;
    glm::vec3 nv = face[nvi].position;

    glm::vec3 s1 = v - pv;
    glm::vec3 s2 = nv - v;
    glm::vec3 s3 = pv - nv;

    // vertex is not convex
    glm::vec3 vert_dir = glm::normalize(glm::cross(s1, s2));
    if(glm::dot(vert_dir, normal) < 0) {
        return false;
    }

    for(int i = 0; i < face_size; i++) {
        if(i != vi && i != nvi && i != pvi) {
            glm::vec3 p = face[i].position;
            glm::vec2 p1x = glm::normalize(glm::cross(s1, p - pv));
            glm::vec2 p2x = glm::normalize(glm::cross(s2, p - v));
            glm::vec2 p3x = glm::normalize(glm::cross(s3, p - nv));

            if(glm::dot(p1x, p2x) > 0 && glm::dot(p2x, p3x) > 0) {
                return false;
            }
        }
    }
    return true;
}

bool loadObj(std::vector<Mesh> &meshes, std::string path) {
    // ensure that the given file is an object file
    // assume it is if it ends in .obj
    if(path.substr(path.size() - 4, 4) != ".obj") {
        std::fprintf(stderr, "File %s is not an object file\n", path.c_str());
        return false;
    }

    std::ifstream model_file(path);

    // ensure the model file exists
    if(!model_file.is_open()) {
        std::fprintf(stderr, "File %s does not exist\n", path.c_str());
        return false;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<Material> materials;

    std::vector<Vertex> current_vertices;
    std::vector<unsigned int> current_indices;
    std::vector<Material> current_materials;
    int face_offset = 0;

    // iterate through each line in the model file
    std::string line;
    while(std::getline(model_file, line)) {
        // get the line type, (AKA the first word of the line)
        int type_len;
        for(type_len = 0; line[type_len] != ' ' && line[type_len] != '\n' &&
                line[type_len] != '\0'; type_len++);
        std::string type = line.substr(0, type_len);

        // line type is a vertex (position) line
        if(type == "v") {
            // parse a posiiton
            std::vector<float> values;
            parseLineFloats(values, line);
            positions.push_back(glm::vec3(values[0], values[1], values[2]));
        }

        // line type is a vertex normal line
        else if(type == "vn") {
            std::vector<float> values;
            parseLineFloats(values, line);
            normals.push_back(glm::vec3(values[0], values[1], values[2]));
        }

        // line type is a texture coordinate line
        else if(type == "vt") {
            // parse a texture coordinate
            std::vector<float> values;
            parseLineFloats(values, line);
            uvs.push_back(glm::vec2(values[0], values[1]));
        }

        // line type is a face line
        else if(type == "f") {
            std::vector<Vertex> face;
            createFace(face, positions, normals, uvs, line);
            glm::vec3 normal = createFaceNormal(face);

            for(int i = 0; i < face.size(); i++) {
                current_vertices.push_back(face[i]);
            }

            int face_size = face.size();
            std::vector<Vertex> face_left = face;

            while(face_left.size() > 2) {
                for(int vi = 0; vi < face_left.size(); vi++) {
                    if(isEar(face_left, normal, vi)) {
                        int pvi = (vi == 0) ? (face_size - 1) : (vi - 1);
                        int nvi = (vi == face_size - 1) ? (0) : (vi + 1);

                        for(int vj = 0; vj < face.size(); vj++) {
                            if(face[vj].position ==
                                    face_left[pvi].position ||
                                    face[vj].position ==
                                    face_left[vi].position ||
                                    face[vj].position ==
                                    face_left[nvi].position) {
                                current_indices.push_back(face_offset + vj);
                            }
                        }

                        face_left.erase(face_left.begin() + vi);

                        break;
                    }
                }
            }

            face_offset += face_size;
        }

        else if(type == "o") {
            // ignore for now
        }

        else if(type == "g") {
            // ignore for now
        }

        else if(type == "s") {
            // ignore for now
        }

        else if(type == "mtllib") {
            std::vector<std::string> name;
            parseLineStrings(name, line);
            std::string folder = path.substr(0, path.rfind('/') + 1); 
            loadMtl(materials, folder + name[0]);
        }

        else if(type == "usemtl") {
            std::vector<std::string> name;
            parseLineStrings(name, line);
            bool found = false;
            for(int i = 0; i < materials.size(); i++) {
                if(name[0] == materials[i].name) {
                    current_materials.push_back(materials[i]);
                    found = true;
                    break;
                }
            }

            if(!found) {
                std::fprintf(stderr, "Could not find material %s\n",
                        name[0].c_str());

                return false;
            }
        }

        // if not any of the above and not a comment, we know the file is
        // improperly formatted
        else if(type != "#") {
            //std::fprintf(stderr, "Object file is improperly formatted\n");

            //return false;
        }
    }

    Mesh m;
    m.create(current_vertices, current_indices, current_materials);
    meshes.push_back(m);

    return true;
}

bool loadMtl(std::vector<Material> &materials, std::string path) {
    // ensure that the given file is an material file
    // assume it is if it ends in .mtl
    if(path.substr(path.size() - 4, 4) != ".mtl") {
        std::fprintf(stderr, "File %s is not a material file\n", path.c_str());
        return false;
    }

    std::ifstream model_file(path);

    // ensure the model file exists
    if(!model_file.is_open()) {
        std::fprintf(stderr, "File %s does not exist\n", path.c_str());
        return false;
    }

    Material current_material;
    current_material.name = "*";

    std::string line;
    while(std::getline(model_file, line)) {
        // get the line type, (AKA the first word of the line)
        int type_len;
        for(type_len = 0; line[type_len] != ' ' && line[type_len] != '\n' &&
                line[type_len] != '\0'; type_len++);
        std::string type = line.substr(0, type_len);

        if(type == "newmtl") {
            if(current_material.name != "*") {
                materials.push_back(current_material);
            }
            std::vector<std::string> name;
            parseLineStrings(name, line);
            current_material.name = name[0];
        }

        else if(type == "map_Kd") {
            std::vector<std::string> name;
            parseLineStrings(name, line);
            std::string folder = path.substr(0, path.rfind('/') + 1); 
            current_material.diffuse.create(folder + name[0]);
        }
    }

    materials.push_back(current_material);

    return true;
}
};
