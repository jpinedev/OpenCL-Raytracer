#pragma once

#include <glm/glm.hpp>

struct LightProperties {
    glm::vec3 ambient{ 0.f,0.f,0.f }, diffuse{ 0.f,0.f,0.f }, specular{ 0.f,0.f,0.f };
};

struct Light : LightProperties {
    glm::vec4 lightPosition{ 0.f, 0.f, 0.f, 1.f };

    Light(const LightProperties& props, glm::mat4 modelview) : LightProperties(props) {
        lightPosition = modelview * lightPosition;
    }

};
