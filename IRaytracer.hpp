#pragma once

#include <glm/glm.hpp>
#include <CL/cl.h>
#include <vector>
#include "HitRecord.hpp"
#include "Light.hpp"
#include "ObjectData.hpp"

class IRaytracer
{
public:
    virtual cl_float4* Render() = 0;

protected:
    const std::vector<ObjectData>& objects;
    const std::vector<Light>& lights;
    const std::vector<Ray3D>& rays;

    IRaytracer(const std::vector<ObjectData>& objects, const std::vector<Light>& lights, const std::vector<Ray3D>& rays) : objects(objects), lights(lights), rays(rays) { }
};

