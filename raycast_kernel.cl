__kernel void raycast(__global const int sizeOfObj, __global const int count, __global const unsigned char* objs, __global float3* rays, __global unsigned char* hits) {
    // Get the index of the current element to be processed
    int i = get_global_id(0);

    hits[i] = 0;

    float3* ray = rays + i * 2;

    float3 rayStart, rayDir;

    // Do the operation

    for (int objIndex = 0; objIndex < count; ++objIndex) {
        unsigned char* obj = objs + objIndex * sizeOfObj;
        float* mvInverse = (float*)(obj + 52U) + 1);

        rayStart.x = mvInverse[0] * ray[0].x + mvInverse[4] * ray[0].y + mvInverse[8] * ray[0].z + mvInverse[12] * 1.f;
        rayStart.y = mvInverse[1] * ray[0].x + mvInverse[5] * ray[0].y + mvInverse[9] * ray[0].z + mvInverse[13] * 1.f;
        rayStart.z = mvInverse[2] * ray[0].x + mvInverse[6] * ray[0].y + mvInverse[10] * ray[0].z + mvInverse[14] * 1.f;

        rayDir.x = mvInverse[0] * ray[1].x + mvInverse[4] * ray[1].y + mvInverse[8] * ray[1].z;
        rayDir.y = mvInverse[1] * ray[1].x + mvInverse[5] * ray[1].y + mvInverse[9] * ray[1].z;
        rayDir.z = mvInverse[2] * ray[1].x + mvInverse[6] * ray[1].y + mvInverse[10] * ray[1].z;

        switch (*(obj + 52U + 3 * 4U * 16U)) {
        case 0: // Sphere
            // Solve quadratic
            float A = rayDir.x * rayDir.x +
                rayDir.y * rayDir.y +
                rayDir.z * rayDir.z;
            float B = 2.f *
                (rayDir.x * rayStart.x + rayDir.y * rayStart.y +
                    rayDir.z * rayStart.z);
            float C = rayStart.x * rayStart.x + rayStart.y * rayStart.y +
                rayStart.z * rayStart.z - 1.f;

            float radical = B * B - 4.f * A * C;
            // no intersection
            if (radical < 0) continue;

            float root = sqrtf(radical);

            float t1 = (-B - root) / (2.f * A);
            float t2 = (-B + root) / (2.f * A);

            float tMin = (t1 >= 0 && t2 >= 0) ? min(t1, t2) : max(t1, t2);
            // object is fully behind camera
            if (tMin < 0) continue;

            hits[i] = 1;
            break;

        case 1: // Box

            break;
        }
    }
}

