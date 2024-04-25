#include <string>
#include <vector>

#include <glad/gl.h>

#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/uniform.h"
#include "graphics/vertex.h"

void Mesh::create(std::vector<Vertex> vertices,
        std::vector<unsigned int> indices, std::vector<Texture> textures) {
    num_indices = indices.size();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
            &vertices[0], GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            (void*) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            (void*) (3 * sizeof(float)));

    this->textures = textures;

    for(int i = 0; i < textures.size(); i++) {
        std::string number = std::to_string(i);
        std::string name = "texture";
        std::string uniform_name = name + number;

        TextureUniform tu;
        tu.name = uniform_name;
        tu.value = i;

        texture_uniforms.push_back(tu);
    }
}

void Mesh::draw() {
    glBindVertexArray(vao);

    for(int i = 0; i < texture_uniforms.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE, textures[i].id);
    }

    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
