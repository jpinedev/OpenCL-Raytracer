typedef struct Ray {
    float4 start;
    float4 direction;
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

const float MAX_FLOAT = 3.402823466e+38F;

bool intersectsWidthBoxSide(float* tMin, float* tMax, float start, float dir) {
    float t1 = (-0.5f - start);
    float t2 = (0.5f - start);
    if (dir == 0) {
        // no intersection
        if (copysign(t1, t2) == t1) return false;

        *tMin = -MAX_FLOAT;
        *tMax = MAX_FLOAT;
        return true;
    }

    t1 /= dir;
    t2 /= dir;

    if (dir < 0) {
        *tMin = fmin(t1, t2);
        *tMax = fmax(t1, t2);
    }
    else {
        *tMin = t1;
        *tMax = t2;
    }

    return true;
}

inline void transform(float4* o_vec, const float16* i_mat, const float4* i_vec) {
    o_vec->x = i_mat->s0 * i_vec->x + i_mat->s4 * i_vec->y + i_mat->s8 * i_vec->z + i_mat->sC * i_vec->w;
    o_vec->y = i_mat->s1 * i_vec->x + i_mat->s5 * i_vec->y + i_mat->s9 * i_vec->z + i_mat->sD * i_vec->w;
    o_vec->z = i_mat->s2 * i_vec->x + i_mat->s6 * i_vec->y + i_mat->sA * i_vec->z + i_mat->sE * i_vec->w;
    o_vec->w = i_mat->s3 * i_vec->x + i_mat->s7 * i_vec->y + i_mat->sB * i_vec->z + i_mat->sF * i_vec->w;
}

__kernel void hittest(const ulong count, __global const ObjectData* objs, __global const Ray* rays, __global float* hits) {
    // Get the index of the current element to be processed
    int i = get_global_id(0);

    Ray ray;

    // Do the operation
    float hit = MAX_FLOAT;

    for (int objIndex = 0; objIndex < count; ++objIndex) {
        const ObjectData* obj = objs + objIndex;

        transform(&ray.start, &obj->mvInverse, &rays[i].start);
        transform(&ray.direction, &obj->mvInverse, &rays[i].direction);

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
        {
            float txMin, txMax, tyMin, tyMax, tzMin, tzMax;

            if (!intersectsWidthBoxSide(&txMin, &txMax, ray.start.x, ray.direction.x))
                continue;

            if (!intersectsWidthBoxSide(&tyMin, &tyMax, ray.start.y, ray.direction.y))
                continue;

            if (!intersectsWidthBoxSide(&tzMin, &tzMax, ray.start.z, ray.direction.z))
                continue;

            float tMin = fmax(fmax(txMin, tyMin), tzMin);
            float tMax = fmin(fmin(txMax, tyMax), tzMax);

            // no intersection
            if (tMax < tMin) continue;

            float tHit = (tMin >= 0 && tMax >= 0) ? fmin(tMin, tMax) : fmax(tMin, tMax);
            // object is fully behind camera
            if (tHit < 0) continue;

            // already hit a closer object
            if (hit <= tHit) continue;

            float4 objSpaceIntersection = { ray.start + tHit * ray.direction };

            float4 objSpaceNormal = { 0.f, 0.f, 0.f, 0.f };
            if (objSpaceIntersection.x > 0.4998f) objSpaceNormal.x += 1.f;
            else if (objSpaceIntersection.x < -0.4998f) objSpaceNormal.x -= 1.f;

            if (objSpaceIntersection.y > 0.4998f) objSpaceNormal.y += 1.f;
            else if (objSpaceIntersection.y < -0.4998f) objSpaceNormal.y -= 1.f;

            if (objSpaceIntersection.z > 0.4998f) objSpaceNormal.z += 1.f;
            else if (objSpaceIntersection.z < -0.4998f) objSpaceNormal.z -= 1.f;

            objSpaceNormal = normalize(objSpaceNormal);

            hit = tHit;
            continue;
        }

        }
    }

    if (hit < MAX_FLOAT) hits[i] = hit;
}

