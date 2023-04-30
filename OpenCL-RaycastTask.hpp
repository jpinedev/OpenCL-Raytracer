#ifndef __RAYCAST_TASK__
#define __RAYCAST_TASK__

#include <vector>
#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include "ObjectData.hpp"

#define MAX_SOURCE_SIZE (0x100000)

using namespace std;


inline void cpyVec3ToFloat3(cl_float3* dest, const glm::vec3& src) {
    *dest = { src.x, src.y, src.z };
}

typedef struct cl_Ray {
    cl_float3 start, direction;

    cl_Ray() : start({0, 0, 0}), direction({0,0,0}) { }
    cl_Ray(const Ray3D& cpy) {
        cpyVec3ToFloat3(&start, cpy.start);
        cpyVec3ToFloat3(&direction, cpy.direction);
    }
} cl_Ray;

typedef struct cl_Material {
    cl_float3 ambient, diffuse, specular;
    cl_float absorption, reflection, transparency;
    cl_float shininess;

    cl_Material() : ambient({0., 0., 0.}), diffuse(ambient), specular(ambient), absorption(1), reflection(0), transparency(0), shininess(1) { }
    cl_Material(const Material& cpy) : absorption(cpy.absorption), reflection(cpy.reflection), transparency(cpy.transparency), shininess(cpy.shininess) {
        cpyVec3ToFloat3(&ambient, cpy.ambient);
        cpyVec3ToFloat3(&diffuse, cpy.diffuse);
        cpyVec3ToFloat3(&specular, cpy.specular);
    }
} cl_Material;

inline void cpyMat4ToFloat16(cl_float16* dest, const glm::mat4& src) {
    const float* m = glm::value_ptr(src);
    *dest = { m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], m[12], m[13], m[14], m[15] };
}

typedef struct cl_ObjectData {
    cl_Material mat;
    cl_float16 mv, mvInverse, mvInverseTranspose;
    cl_uint type;
    // Necessary for alignment on vram (for my drivers)
    uint8_t spacer[60];

    cl_ObjectData() : mv({ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }), mvInverse(mv), mvInverseTranspose(mv), type(0) { }
    cl_ObjectData(const ObjectData& cpy) : mat(cpy.mat) {
        cpyMat4ToFloat16(&mv, cpy.mv);
        cpyMat4ToFloat16(&mvInverse, cpy.mvInverse);
        cpyMat4ToFloat16(&mvInverseTranspose, cpy.mvInverseTranspose);
        type = (cl_uint)cpy.type;
    }
} cl_ObjectData;

static int raycastRays(const vector<ObjectData>& objects, const vector<Ray3D>& rays, vector<vector<HitRecord> >& hits) {
    const size_t OBJECT_COUNT = objects.size();
    cl_ObjectData* objArr = new cl_ObjectData[OBJECT_COUNT];

    for (int i = 0; i < OBJECT_COUNT; i++) {
        objArr[i] = cl_ObjectData(objects[i]);
    }

    const size_t RAYCAST_COUNT = rays.size();

    cl_Ray* rayArr = new cl_Ray[RAYCAST_COUNT];

    float* hitTestArr = new float[RAYCAST_COUNT];

    for (int i = 0; i < RAYCAST_COUNT; i++) {
        rayArr[i] = cl_Ray(rays[i]);

        hitTestArr[i] = 0.f;
    }

    // Load the kernel source code into the array source_str
    FILE* fp;
    char* source_str;
    size_t source_size;

    fp = fopen("raycast_kernel.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = new char[MAX_SOURCE_SIZE];
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    // Get platform and device information
    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1,
        &device_id, &ret_num_devices);

    // Create an OpenCL context
    cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);

    // Create a command queue
    cl_command_queue command_queue = clCreateCommandQueueWithProperties(context, device_id, 0, &ret);

    // Create memory buffers on the device for each vector 
    cl_mem objs_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
        OBJECT_COUNT * sizeof(cl_ObjectData), NULL, &ret);
    cl_mem rays_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
        RAYCAST_COUNT * sizeof(cl_Ray), NULL, &ret);
    cl_mem hits_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
        RAYCAST_COUNT * sizeof(float), NULL, &ret);

    // Copy the lists A and B to their respective memory buffers
    ret = clEnqueueWriteBuffer(command_queue, objs_mem_obj, CL_TRUE, 0,
        OBJECT_COUNT * sizeof(cl_ObjectData), objArr, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, rays_mem_obj, CL_TRUE, 0,
        RAYCAST_COUNT * sizeof(cl_Ray), rayArr, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, hits_mem_obj, CL_TRUE, 0,
        RAYCAST_COUNT * sizeof(float), hitTestArr, 0, NULL, NULL);

    // Create a program from the kernel source
    cl_program program = clCreateProgramWithSource(context, 1,
        (const char**)&source_str, (const size_t*)&source_size, &ret);

    // Build the program
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    cl_build_status status;
    ret = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &status, NULL);
    if (ret != 0 || status != CL_BUILD_SUCCESS) {
        std::cout << ret << endl;

        size_t s;
        ret = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, MAX_SOURCE_SIZE, source_str, &s);
        std::cout << source_str;
        throw;
    }

    // Create the OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, "raycast", &ret);

    // Set the arguments of the kernel
    ret = clSetKernelArg(kernel, 0, sizeof(size_t), &OBJECT_COUNT);
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&objs_mem_obj);
    ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&rays_mem_obj);
    ret = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void*)&hits_mem_obj);

    // Execute the OpenCL kernel on the list
    size_t global_item_size = RAYCAST_COUNT; // Process the entire lists
    size_t local_item_size = 32; // Divide work items into groups of 64
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
        &global_item_size, &local_item_size, 0, NULL, NULL);

    // Read the memory buffer C on the device to the local variable C
    ret = clEnqueueReadBuffer(command_queue, hits_mem_obj, CL_TRUE, 0,
        RAYCAST_COUNT * sizeof(float), hitTestArr, 0, NULL, NULL);

    for (int i = 0; i < RAYCAST_COUNT; i++) {
        if (hitTestArr[i] > 0U)
            hits[i / hits.size()][i % hits.size()].time = hitTestArr[i];
    }

    // Clean up
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(objs_mem_obj);
    ret = clReleaseMemObject(rays_mem_obj);
    ret = clReleaseMemObject(hits_mem_obj);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);
    delete[] objArr;
    delete[] rayArr;
    delete[] hitTestArr;
    return 0;
}

#endif