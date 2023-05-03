#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "HitRecord.hpp"
#include "Light.hpp"
#include "ObjectData.hpp"

class IRaytracer
{
public:
    virtual void Render(std::vector<float>& pixelData) = 0;

protected:
    const std::vector<ObjectData>& objects;
    const std::vector<Light>& lights;
    const std::vector<Ray3D>& rays;

    IRaytracer(const std::vector<ObjectData>& objects, const std::vector<Light>& lights, const std::vector<Ray3D>& rays) : objects(objects), lights(lights), rays(rays) { }
};

