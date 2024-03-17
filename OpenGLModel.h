#pragma once
#include <glad/glad.h>
#include <vector>
#include "Material.hpp"
#include "ObjectData.hpp"
#include "Light.hpp"

struct GLMaterial
{
    GLfloat ambient[3], diffuse[3], specular[3];
    GLfloat absorption, reflection, transparency;
    GLfloat shininess;

    GLMaterial(const Material& mat);
};

struct GLObjectData
{
    GLfloat mv[16], mvInverse[16], mvInvTranspose[16];
    GLMaterial mat;
    GLuint type;

    GLObjectData(const ObjectData& obj);
};

struct GLLight
{
    GLfloat position[4];
    GLfloat ambient[3], diffuse[3], specular[3];

    GLLight(const Light& light);
};

struct OpenGLModel
{
    OpenGLModel(const float maxBounces, const std::vector<ObjectData> objs, const std::vector<Light> lights);

    const float MAX_BOUNCES;
    const std::vector<ObjectData> objs;
    const std::vector<Light> lights;
};

