#pragma once

#include <glm/glm.hpp>

class Light
{
    glm::vec3 ambient, diffuse, specular;
    glm::vec4 lightPosition{0.f, 0.f, 0.f, 1.f};
};
