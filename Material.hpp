#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Material
{
    glm::vec3 ambient{ 0.f,0.f,0.f }, diffuse{ 0.f,0.f,0.f }, specular{ 0.f,0.f,0.f };
    float absorption = 1, reflection = 0, transparency = 0;
    float shininess = 1;

    void Serialize(uint8_t*& dest) const {
        memcpy(dest, glm::value_ptr(ambient), 3 * sizeof(float));
        dest += 3 * sizeof(float);
        memcpy(dest, glm::value_ptr(diffuse), 3 * sizeof(float));
        dest += 3 * sizeof(float);
        memcpy(dest, glm::value_ptr(specular), 3 * sizeof(float));
        dest += 3 * sizeof(float);
        memcpy(dest, &absorption, sizeof(float));
        dest += sizeof(float);
        memcpy(dest, &reflection, sizeof(float));
        dest += sizeof(float);
        memcpy(dest, &transparency, sizeof(float));
        dest += sizeof(float);
        memcpy(dest, &shininess, sizeof(float));
        dest += sizeof(float);
    }
    // static Material Deserialize(uint8_t*& src);
};

