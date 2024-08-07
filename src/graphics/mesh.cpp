#include <stdio.h>
#include <string>
#include <vector>

#include <glad/gl.h>

#include "graphics/material.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/vertex.h"

void Mesh::create(std::vector<Vertex> vertices,
        std::vector<unsigned int> indices, std::vector<Material> materials) {
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
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            (void*) (6 * sizeof(float)));

    this->materials = materials;
}

void Mesh::draw(ShaderProgram const &shader) {
    glBindVertexArray(vao);

    for(int i = 0; i < materials.size(); i += 3) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE, materials[i].ambient.id);
        glActiveTexture(GL_TEXTURE0 + i + 1);
        glBindTexture(GL_TEXTURE, materials[i].diffuse.id);
        glActiveTexture(GL_TEXTURE0 + i + 2);
        glBindTexture(GL_TEXTURE, materials[i].specular.id);
    }

    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::destroy() {
    for(int i = 0; i < materials.size(); i++) {
        materials[i].ambient.destroy();
        materials[i].diffuse.destroy();
        materials[i].specular.destroy();
    }
}
