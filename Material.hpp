#pragma once

#include <glm/glm.hpp>

struct Material
{
    glm::vec3 ambient{ 0.f,0.f,0.f }, diffuse{ 0.f,0.f,0.f }, specular{ 0.f,0.f,0.f };
    float absorption = 1, reflection = 0, transparency = 0;
    float shininess = 1;

    // void Serialize(void* dest);
    // static Material Deserialize(void* src);
};

