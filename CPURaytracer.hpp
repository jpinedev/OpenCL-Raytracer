#pragma once

#include "IRaytracer.hpp"

class CPURaytracer : IRaytracer
{

public:
    // Inherited via IRaytracer
    virtual int Raytrace(const std::vector<ObjectData>& objects, const std::vector<Ray3D>& rays, std::vector<HitRecord>& hits) override;
};

