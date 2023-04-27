#pragma once

#include <glm/glm.hpp>

struct Material
{
    glm::vec3 ambient, diffuse, specular;
    float absorption = 1, reflection = 0, transparency = 0;
    float shininess = 1;

    // void Serialize(void* dest);
    // static Material Deserialize(void* src);
};

