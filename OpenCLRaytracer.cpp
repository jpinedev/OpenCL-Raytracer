#include "OpenCLRaytracer.hpp"

#include <iostream>
#include <chrono>
#include <stdio.h>

#include <windows.h>
#include <boost/compute/system.hpp>
#include <boost/compute/buffer.hpp>

using namespace std;

OpenCLRaytracer::OpenCLRaytracer(const std::vector<ObjectData>& objects, const std::vector<Light>& lights, const std::vector<Ray3D>& rays, const unsigned int MAX_BOUNCES)
    : IRaytracer(objects, lights, rays), MAX_BOUNCES(MAX_BOUNCES), OBJECT_COUNT(objects.size()), LIGHT_COUNT(lights.size()), RAYCAST_COUNT(rays.size())
{
    objArr.reserve((size_t)OBJECT_COUNT);
    for (int i = 0; i < OBJECT_COUNT; i++) {
        objArr.emplace_back(objects[i]);
    }

    lightArr.reserve((size_t)LIGHT_COUNT);
    for (int i = 0; i < LIGHT_COUNT; i++) {
        lightArr.emplace_back(lights[i]);
    }

    rayArr.reserve((size_t)RAYCAST_COUNT);

    pixelDataArr = new cl_float3[(size_t)RAYCAST_COUNT];
    for (int i = 0; i < RAYCAST_COUNT; i++) {
        rayArr.emplace_back(rays[i]);

        pixelDataArr[i] = { 0.f, 0.f, 0.f, 1.f };
    }

    // Get platform and device information
    boost::compute::device gpu = boost::compute::system::default_device();

    // Create an OpenCL context
    //context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
    context = boost::compute::system::default_context();
    //context = boost::compute::opengl_create_shared_context();

    // Create a command queue
    command_queue = boost::compute::system::default_queue();

    // Create memory buffers on the device for each vector 
    objs_mem_obj = boost::compute::buffer(context, (size_t)OBJECT_COUNT * sizeof(cl_ObjectData), CL_MEM_READ_ONLY);
    lights_mem_obj = boost::compute::buffer(context, (size_t)LIGHT_COUNT * sizeof(cl_Light), CL_MEM_READ_ONLY);
    rays_mem_obj = boost::compute::buffer(context, (size_t)RAYCAST_COUNT * sizeof(cl_Ray), CL_MEM_READ_ONLY);
    pixelData_mem_obj = boost::compute::buffer(context, (size_t)RAYCAST_COUNT * sizeof(cl_ObjectData), CL_MEM_WRITE_ONLY);

    // Create a program from the kernel source
    program = boost::compute::program::create_with_source_file("shade_and_reflect_kernel.cl", context);

    // Build the program
    program.build();

    // Create the OpenCL kernel
    kernel = program.create_kernel("shade_and_reflect");

    // Set the arguments of the kernel
    kernel.set_arg(0, sizeof(cl_uint), &MAX_BOUNCES);
    kernel.set_arg(1, sizeof(cl_uint), &OBJECT_COUNT);
    kernel.set_arg(2, sizeof(cl_mem), (void*)&objs_mem_obj);
    kernel.set_arg(3, sizeof(cl_uint), &LIGHT_COUNT);
    kernel.set_arg(4, sizeof(cl_mem), (void*)&lights_mem_obj);
    kernel.set_arg(5, sizeof(cl_mem), (void*)&rays_mem_obj);
    kernel.set_arg(6, sizeof(cl_mem), (void*)&pixelData_mem_obj);

    command_queue.enqueue_write_buffer(objs_mem_obj, 0, (size_t)OBJECT_COUNT * sizeof(cl_ObjectData), objArr.data());
    command_queue.enqueue_write_buffer(lights_mem_obj, 0, (size_t)LIGHT_COUNT * sizeof(cl_Light), lightArr.data());
    command_queue.enqueue_write_buffer(rays_mem_obj, 0, (size_t)RAYCAST_COUNT * sizeof(cl_Ray), rayArr.data());
    command_queue.enqueue_write_buffer(pixelData_mem_obj, 0, (size_t)RAYCAST_COUNT * sizeof(cl_float3), pixelDataArr);
}

OpenCLRaytracer::~OpenCLRaytracer() {

}

cl_float4* OpenCLRaytracer::Render()
{
#if _DEBUG
    std::cout << "Executing kernel...\n";

    auto startTime = std::chrono::high_resolution_clock::now();
#endif

    // Execute the OpenCL kernel on the list
    size_t global_item_size = (size_t)RAYCAST_COUNT; // Process the entire lists
    size_t local_item_size = 32; // Divide work items into groups of 64
    command_queue.enqueue_1d_range_kernel(kernel, 0, global_item_size, local_item_size);

    // Read the memory buffer C on the device to the local variable C
    command_queue.enqueue_read_buffer(pixelData_mem_obj, 0, (size_t)RAYCAST_COUNT * sizeof(cl_float3), pixelDataArr);

#if _DEBUG
    auto endTime = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    std::cout << "Kernel finished in " << duration.count() << "ms.\n";
#endif

    return pixelDataArr;
}


inline void cpyVec3ToFloat3(cl_float3* dest, const glm::vec3& src) {
    *dest = { src.x, src.y, src.z };
}
inline void cpyVec4ToFloat4(cl_float4* dest, const glm::vec4& src) {
    *dest = { src.x, src.y, src.z, src.w };
}
inline void cpyMat4ToFloat16(cl_float16* dest, const glm::mat4& src) {
    const float* m = glm::value_ptr(src);
    *dest = { m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], m[12], m[13], m[14], m[15] };
}

OpenCLRaytracer::cl_Ray::cl_Ray() : start({ 0, 0, 0 }), direction({ 0,0,0 }) { }
OpenCLRaytracer::cl_Ray::cl_Ray(const Ray3D& cpy) {
    cpyVec4ToFloat4(&start, cpy.start);
    cpyVec4ToFloat4(&direction, cpy.direction);
}

OpenCLRaytracer::cl_Material::cl_Material() : ambient({ 0., 0., 0. }), diffuse(ambient), specular(ambient), absorption(1), reflection(0), transparency(0), shininess(1) { }
OpenCLRaytracer::cl_Material::cl_Material(const Material& cpy) : absorption(cpy.absorption), reflection(cpy.reflection), transparency(cpy.transparency), shininess(cpy.shininess) {
    cpyVec3ToFloat3(&ambient, cpy.ambient);
    cpyVec3ToFloat3(&diffuse, cpy.diffuse);
    cpyVec3ToFloat3(&specular, cpy.specular);
}

OpenCLRaytracer::cl_ObjectData::cl_ObjectData() : mv({ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }), mvInverse(mv), mvInverseTranspose(mv), type(0) { }
OpenCLRaytracer::cl_ObjectData::cl_ObjectData(const ObjectData& cpy) : mat(cpy.mat) {
    cpyMat4ToFloat16(&mv, cpy.mv);
    cpyMat4ToFloat16(&mvInverse, cpy.mvInverse);
    cpyMat4ToFloat16(&mvInverseTranspose, cpy.mvInverseTranspose);
    type = (cl_uint)cpy.type;
}

OpenCLRaytracer::cl_Light::cl_Light() : ambient({ 0., 0., 0. }), diffuse(ambient), specular(ambient), position({ 0., 0., 0., 1. }) { }
OpenCLRaytracer::cl_Light::cl_Light(const Light& cpy) {
    cpyVec3ToFloat3(&ambient, cpy.ambient);
    cpyVec3ToFloat3(&diffuse, cpy.diffuse);
    cpyVec3ToFloat3(&specular, cpy.specular);
    cpyVec4ToFloat4(&position, cpy.lightPosition);
}
