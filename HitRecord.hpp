#pragma once

#include <limits>
#include <glm/glm.hpp>

#include "Material.hpp"

const static float MAX_FLOAT = std::numeric_limits<float>::max();

struct HitRecord {
    Material mat;
    glm::vec3 intersection;
    glm::vec3 normal;
    float time = MAX_FLOAT;
};