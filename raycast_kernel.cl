typedef struct Ray {
    float3 start;
    float3 direction;
} Ray;

typedef struct Material {
    float3 ambient, diffuse, specular;
    float absorption, reflection, transparency;
    float shininess;
} Material;

typedef struct ObjectData {
    Material mat;
    float16 mv, mvInverse, mvInverseTranspose;
    uint type;
} ObjectData;

__kernel void raycast(const ulong count, __global const ObjectData* objs, __global const Ray* rays, __global float* hits) {
    // Get the index of the current element to be processed
    int i = get_global_id(0);

    Ray ray;

    // Do the operation
    float hit = 3.402823466e+38F;

    for (int objIndex = 0; objIndex < count; ++objIndex) {
        const ObjectData* obj = objs + objIndex;

        ray.start.x = obj->mvInverse.s0 * rays[i].start.x + obj->mvInverse.s4 * rays[i].start.y + obj->mvInverse.s8 * rays[i].start.z + obj->mvInverse.sC * 1.f;
        ray.start.y = obj->mvInverse.s1 * rays[i].start.x + obj->mvInverse.s5 * rays[i].start.y + obj->mvInverse.s9 * rays[i].start.z + obj->mvInverse.sD * 1.f;
        ray.start.z = obj->mvInverse.s2 * rays[i].start.x + obj->mvInverse.s6 * rays[i].start.y + obj->mvInverse.sA * rays[i].start.z + obj->mvInverse.sE * 1.f;

        ray.direction.x = obj->mvInverse.s0 * rays[i].direction.x + obj->mvInverse.s4 * rays[i].direction.y + obj->mvInverse.s8 * rays[i].direction.z;
        ray.direction.y = obj->mvInverse.s1 * rays[i].direction.x + obj->mvInverse.s5 * rays[i].direction.y + obj->mvInverse.s9 * rays[i].direction.z;
        ray.direction.z = obj->mvInverse.s2 * rays[i].direction.x + obj->mvInverse.s6 * rays[i].direction.y + obj->mvInverse.sA * rays[i].direction.z;

        switch (obj->type) {
        case 0: // Sphere
        {
            // Solve quadratic
            float A = ray.direction.x * ray.direction.x +
                ray.direction.y * ray.direction.y +
                ray.direction.z * ray.direction.z;
            float B = 2.f *
                (ray.direction.x * ray.start.x + ray.direction.y * ray.start.y +
                    ray.direction.z * ray.start.z);
            float C = ray.start.x * ray.start.x + ray.start.y * ray.start.y +
                ray.start.z * ray.start.z - 1.f;

            float radical = B * B - 4.f * A * C;

            // no intersection
            if (radical < 0) continue;

            float root = sqrt(radical);

            float t1 = (-B - root) / (2.f * A);
            float t2 = (-B + root) / (2.f * A);

            float tMin = (t1 >= 0 && t2 >= 0) ? fmin(t1, t2) : fmax(t1, t2);
            // object is fully behind camera
            if (tMin < 0) continue;

            if (hit < tMin) continue;
            
            hit = tMin;
            continue;
        }

        case 1: // Box
            break;
        }
    }

    if (hit < 3.402823466e+38F) hits[i] = hit;
}

