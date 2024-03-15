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

#include <boost/compute/system.hpp>
#include <boost/compute/buffer.hpp>
#include <boost/compute/context.hpp>
#include <boost/compute/command_queue.hpp>
#include <boost/compute/program.hpp>
#include <boost/compute/kernel.hpp>

#define MAX_SOURCE_SIZE (0x100000)

class OpenCLRaytracer : IRaytracer {
    struct cl_Ray {
        cl_float4 start, direction;

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

    struct cl_Light {
        cl_float3 ambient, diffuse, specular;
        cl_float4 position;

        cl_Light();
        cl_Light(const Light& cpy);
    };

public:
    OpenCLRaytracer(const std::vector<ObjectData>& objects, const std::vector<Light>& lights, const std::vector<Ray3D>& rays, const unsigned int MAX_BOUNCES);
    ~OpenCLRaytracer();

    // Inherited via IRaytracer
    virtual cl_float4* Render() override;

private:
    const cl_uint MAX_BOUNCES;
    const cl_uint OBJECT_COUNT, LIGHT_COUNT, RAYCAST_COUNT;

    std::vector<cl_ObjectData> objArr;
    std::vector<cl_Light> lightArr;
    std::vector<cl_Ray> rayArr;
    cl_float4* pixelDataArr = NULL;

    boost::compute::buffer objs_mem_obj;
    boost::compute::buffer lights_mem_obj;
    boost::compute::buffer rays_mem_obj;
    boost::compute::buffer pixelData_mem_obj;

    boost::compute::context context;
    boost::compute::command_queue command_queue;
    boost::compute::program program;
    boost::compute::kernel kernel;
};

#endif