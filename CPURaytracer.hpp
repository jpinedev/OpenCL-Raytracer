#pragma once

#include "IRaytracer.hpp"

class CPURaytracer : IRaytracer
{

public:
    // Inherited via IRaytracer
    virtual int HitTest(const std::vector<ObjectData>& objects, const std::vector<Ray3D>& rays, std::vector<HitRecord>& hits, std::vector<glm::vec3>& pixelData) override;

    // Inherited via IRaytracer
    virtual int Shade(const std::vector<ObjectData>& objects, const std::vector<Light>& lights, const std::vector<Ray3D>& rays, std::vector<HitRecord>& hits, std::vector<glm::vec3>& pixelData) override;

    // Inherited via IRaytracer
    virtual int ShadeWithReflections(const unsigned int MAX_BOUNCES, const std::vector<ObjectData>& objects, const std::vector<Light>& lights, const std::vector<Ray3D>& rays, std::vector<HitRecord>& hits, std::vector<glm::vec3>& pixelData) override;
};

