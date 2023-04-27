#pragma once

#include <glm/glm.hpp>

struct Ray3D {
    glm::vec4 start;
    glm::vec4 direction;

    Ray3D(glm::vec3 start, glm::vec3 direction) : start(start, 1.f), direction(direction, 0.f) { }
};