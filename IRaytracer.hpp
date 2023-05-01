#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "HitRecord.hpp"
#include "Light.hpp"
#include "ObjectData.hpp"

class IRaytracer
{
public:
    virtual int HitTest(const std::vector<ObjectData>& objects, const std::vector<Ray3D>& rays, std::vector<HitRecord>& hits, std::vector<glm::vec3>& pixelData) = 0;
    virtual int Shade(const std::vector<ObjectData>& objects, const std::vector<Light>& lights, const std::vector<Ray3D>& rays, std::vector<HitRecord>& hits, std::vector<glm::vec3>& pixelData) = 0;
    virtual int ShadeWithReflections(const unsigned int MAX_BOUNCES, const std::vector<ObjectData>& objects, const std::vector<Light>& lights, const std::vector<Ray3D>& rays, std::vector<HitRecord>& hits, std::vector<glm::vec3>& pixelData) = 0;

protected:
    IRaytracer() { }
};

