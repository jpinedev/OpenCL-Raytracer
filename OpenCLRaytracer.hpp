#ifndef __RAYCAST_TASK__
#define __RAYCAST_TASK__

#include <vector>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include "IRaytracer.hpp"
#include "ObjectData.hpp"

#define MAX_SOURCE_SIZE (0x100000)

class OpenCLRaytracer : IRaytracer {
    struct cl_Ray {
        cl_float3 start, direction;

        cl_Ray();
        cl_Ray(const Ray3D& cpy);
    };

    struct cl_Material {
        cl_float3 ambient, diffuse, specular;
        cl_float absorption, reflection, transparency;
        cl_float shininess;

        cl_Material();
        cl_Material(const Material& cpy);
    };

    struct cl_ObjectData {
        cl_Material mat;
        cl_float16 mv, mvInverse, mvInverseTranspose;
        cl_uint type;
        // Necessary for alignment on vram (for my drivers)
        uint8_t spacer[60];

        cl_ObjectData();
        cl_ObjectData(const ObjectData& cpy);
    };

public:
    // Inherited via IRaytracer
    virtual int Raytrace(const std::vector<ObjectData>& objects, const std::vector<Ray3D>& rays, std::vector<HitRecord>& hits) override;
};

#endif