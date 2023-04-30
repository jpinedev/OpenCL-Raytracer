#include "CPURaytracer.hpp"

int CPURaytracer::Raytrace(const std::vector<ObjectData>& objects, const std::vector<Ray3D>& rays, std::vector<HitRecord>& hits)
{
    for (int ii = 0; ii < rays.size(); ++ii) {
        for (auto& obj : objects) {
            obj.Raycast(rays[ii], hits[ii]);
        }
    }
    return 0;
}
