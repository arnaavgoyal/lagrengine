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
    Model model;
    glm::mat4 world;
};

struct Scene {

    std::list<SceneObject *> objects;
    std::map<ShaderProgram, std::list<SceneObject>> shader_obj_map;

    SceneObject &addObject(Model m, glm::mat4 wld, ShaderProgram s) {
        decltype(shader_obj_map)::iterator iter;
        std::list<SceneObject> &list = shader_obj_map[s];
        SceneObject &so = list.emplace_back();
        so = {m, wld};
        objects.push_back(&so);
        return so;
    }

    void draw(Camera &cam) {
        // calculate view matrix
        glm::mat4 view = glm::lookAt(
            cam.pos,
            cam.pos + cam.front,
            cam.up
        );

        for (auto &[shader, objs] : shader_obj_map) {

            glUseProgram(shader.id);
                glUniformMatrix4fv(
                glGetUniformLocation(shader.id, "view"),
                1,
                GL_FALSE,
                glm::value_ptr(view)
            );

            glUniformMatrix4fv(
                glGetUniformLocation(shader.id, "proj"),
                1,
                GL_FALSE,
                glm::value_ptr(cam.proj)
            );

            for (auto &obj : objs) {
                
                glUniformMatrix4fv(
                    glGetUniformLocation(shader.id, "model"),
                    1,
                    GL_FALSE,
                    glm::value_ptr(obj.world)
                );

                obj.model.draw(shader);
            }
        }
    }
};

#endif
