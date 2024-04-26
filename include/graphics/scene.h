#ifndef GRAPHICS_SCENE_H
#define GRAPHICS_SCENE_H

#include <list>
#include <map>
#include <vector>

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>

#include "graphics/camera.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"

struct SceneObject {
    Mesh mesh;
    glm::mat4 world;
};

struct Scene {

    std::list<SceneObject *> objects;
    std::map<ShaderProgram, std::list<SceneObject>> shader_obj_map;

    SceneObject &addObject(Mesh m, glm::mat4 wld, ShaderProgram s) {
        decltype(shader_obj_map)::iterator iter;
        std::list<SceneObject> &list = shader_obj_map[s];
        SceneObject &so = list.emplace_back();
        so = { m, wld };
        objects.push_back(&so);
        return so;
    }

    void draw(Camera cam) {

        // set the viewport
        glViewport(0, 0, cam.width, cam.height);

        // calculate view matrix
        glm::mat4 view = glm::lookAt(
            cam.pos,
            cam.pos + cam.front,
            cam.up
        );

        // calculate proj matrix
        glm::mat4 proj = glm::perspective(
            glm::radians(cam.fov),
            (float)cam.width / (float)cam.height,
            0.1f,
            100.0f
        );

        for (auto &[shader, objs] : shader_obj_map) {

            glUseProgram(shader);
                glUniformMatrix4fv(
                glGetUniformLocation(shader, "view"),
                1,
                GL_FALSE,
                glm::value_ptr(view)
            );

            glUniformMatrix4fv(
                glGetUniformLocation(shader, "proj"),
                1,
                GL_FALSE,
                glm::value_ptr(proj)
            );

            for (auto &obj : objs) {
                
                glUniformMatrix4fv(
                    glGetUniformLocation(shader, "model"),
                    1,
                    GL_FALSE,
                    glm::value_ptr(obj.world)
                );

                for(TextureUniform tu : obj.mesh.texture_uniforms) {
                    glUniform1i(glGetUniformLocation(shader, tu.name.c_str()), tu.value);
                }

                obj.mesh.draw();
            }
        }
    }
};

#endif
