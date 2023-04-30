#pragma once

#include <vector>
#include "HitRecord.hpp"
#include "ObjectData.hpp"

class IRaytracer
{
public:
    virtual int Raytrace(const std::vector<ObjectData>& objects, const std::vector<Ray3D>& rays, std::vector<HitRecord>& hits) = 0;

protected:
    IRaytracer() { }
};

