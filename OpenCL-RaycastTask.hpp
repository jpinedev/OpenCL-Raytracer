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

static int raycastRays(const vector<ObjectData>& objects, const vector<Ray3D>& rays, vector<vector<HitRecord> >& hits) {
    const size_t OBJECT_COUNT = objects.size();
    uint8_t* objArr = new uint8_t[OBJECT_COUNT * ObjectData::SERIALIZED_SIZE];
    uint8_t* currObj = objArr;

    for (int i = 0; i < OBJECT_COUNT; i++) {
        objects[i].Serialize(currObj);
    }

    const size_t RAYCAST_COUNT = rays.size();

    const size_t RAY_LIST_SIZE = RAYCAST_COUNT * 6;
    float* rayArr = new float[RAYCAST_COUNT * 6];

    const size_t HITTEST_LIST_SIZE = RAYCAST_COUNT * sizeof(uint8_t);
    uint8_t* hitTestArr = new uint8_t[RAYCAST_COUNT];

    for (int i = 0; i < RAYCAST_COUNT; i++) {
        memcpy(rayArr + i * 6, glm::value_ptr(rays[i].start), 3 * sizeof(float));
        memcpy(rayArr + i * 6 + 3, glm::value_ptr(rays[i].direction), 3 * sizeof(float));

        hitTestArr[i] = 0;
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
        OBJECT_COUNT * ObjectData::SERIALIZED_SIZE, NULL, &ret);
    cl_mem rays_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
        RAY_LIST_SIZE * sizeof(float), NULL, &ret);
    cl_mem hits_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
        HITTEST_LIST_SIZE * sizeof(uint8_t), NULL, &ret);

    // Copy the lists A and B to their respective memory buffers
    ret = clEnqueueWriteBuffer(command_queue, objs_mem_obj, CL_TRUE, 0,
        OBJECT_COUNT * ObjectData::SERIALIZED_SIZE, objArr, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, rays_mem_obj, CL_TRUE, 0,
        RAY_LIST_SIZE * sizeof(float), rayArr, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, hits_mem_obj, CL_TRUE, 0,
        HITTEST_LIST_SIZE * sizeof(uint8_t), hitTestArr, 0, NULL, NULL);

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
    }

    // Create the OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, "raycast", &ret);

    // Set the arguments of the kernel
    ret = clSetKernelArg(kernel, 0, sizeof(size_t), &ObjectData::SERIALIZED_SIZE);
    ret = clSetKernelArg(kernel, 1, sizeof(size_t), &OBJECT_COUNT);
    ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&objs_mem_obj);
    ret = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void*)&rays_mem_obj);
    ret = clSetKernelArg(kernel, 4, sizeof(cl_mem), (void*)&hits_mem_obj);

    // Execute the OpenCL kernel on the list
    size_t global_item_size = RAYCAST_COUNT; // Process the entire lists
    size_t local_item_size = 64; // Divide work items into groups of 64
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
        &global_item_size, &local_item_size, 0, NULL, NULL);

    // Read the memory buffer C on the device to the local variable C
    ret = clEnqueueReadBuffer(command_queue, hits_mem_obj, CL_TRUE, 0,
        RAYCAST_COUNT * sizeof(uint8_t), hitTestArr, 0, NULL, NULL);

    for (int i = 0; i < RAYCAST_COUNT; i++) {
        if (hitTestArr[i] > 0U)
            hits[i / hits.size()][i % hits.size()].time = 1.f;
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