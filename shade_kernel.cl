typedef struct Ray {
    float4 start;
    float4 direction;
} Ray;

typedef struct Material {
    float3 ambient, diffuse, specular;
    float absorption, reflection, transparency;
    float shininess;
} Material;

typedef struct HitRecord {
    Material mat;
    float4 intersection;
    float3 normal;
    float time;
} HitRecord;

typedef struct ObjectData {
    Material mat;
    float16 mv, mvInverse, mvInverseTranspose;
    uint type;
} ObjectData;

typedef struct Light {
    float3 ambient, diffuse, specular;
    float4 position;
} Light;

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

bool raycast(const ulong count, const ObjectData* objs, const Ray* viewspaceRay, HitRecord* hit) {
    Ray ray;

    for (int objIndex = 0; objIndex < count; ++objIndex) {
        const ObjectData* obj = objs + objIndex;

        transform(&ray.start, &obj->mvInverse, &viewspaceRay->start);
        transform(&ray.direction, &obj->mvInverse, &viewspaceRay->direction);

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

            if (hit->time < tMin) continue;

            hit->time = tMin;

            float4 objSpaceIntersection = { ray.start + tMin * ray.direction };
            transform(&hit->intersection, &obj->mv, &objSpaceIntersection);
            float4 objSpaceNormal = objSpaceIntersection;
            objSpaceNormal.w = 0.f;
            float4 normal;
            transform(&normal, &obj->mv, &objSpaceNormal);
            hit->normal = normalize(normal.xyz);
            hit->mat = obj->mat;
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
            if (hit->time <= tHit) continue;

            float4 objSpaceIntersection = { ray.start + tHit * ray.direction };

            float4 objSpaceNormal = { 0.f, 0.f, 0.f, 0.f };
            if (objSpaceIntersection.x > 0.4998f) objSpaceNormal.x += 1.f;
            else if (objSpaceIntersection.x < -0.4998f) objSpaceNormal.x -= 1.f;

            if (objSpaceIntersection.y > 0.4998f) objSpaceNormal.y += 1.f;
            else if (objSpaceIntersection.y < -0.4998f) objSpaceNormal.y -= 1.f;

            if (objSpaceIntersection.z > 0.4998f) objSpaceNormal.z += 1.f;
            else if (objSpaceIntersection.z < -0.4998f) objSpaceNormal.z -= 1.f;

            hit->time = tHit;
            transform(&hit->intersection, &obj->mv, &objSpaceIntersection);
            float4 normal;
            transform(&normal, &obj->mv, &objSpaceNormal);
            hit->normal = normalize(normal.xyz);
            hit->mat = obj->mat;
            continue;
        }

        }
    }

    return (hit->time < MAX_FLOAT);
}

inline float3 componentWiseMultiply(const float3 lhs, const float3 rhs)
{
    return (float3)(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
}

// assumes normal is normalized
inline float3 reflect(const float3 incident, const float3 normal) {
    return incident - 2.f * dot(incident, normal) * normal;
}

__kernel void shade(const uint MAX_BOUNCES, const ulong OBJECT_COUNT, __global const ObjectData* objs, const ulong LIGHT_COUNT, __global const Light* lights, __global const Ray* rays, __global float3* pixelData) {
    // Get the index of the current element to be processed
    int ii = get_global_id(0);

    HitRecord hit;
    hit.time = MAX_FLOAT;

    if (!raycast(OBJECT_COUNT, objs, &rays[ii], &hit)) return;

    // shade using normals
    //pixelData[ii] = (hit.normal + (float3)( 1.f, 1.f, 1.f )) * 0.5f;
    //return;

    // shade using ambient
    //pixelData[ii] = hit.mat.ambient;
    //return;

    float3 fPosition = hit.intersection.xyz;
    float3 fNormal = hit.normal;
    float3 fColor = { 0.f, 0.f, 0.f };
    float3 lightVec = { 0.f, 0.f, 0.f }, viewVec = { 0.f, 0.f, 0.f }, reflectVec = { 0.f, 0.f, 0.f };
    float3 normalView = { 0.f, 0.f, 0.f };
    float3 ambient = { 0.f, 0.f, 0.f }, diffuse = { 0.f, 0.f, 0.f }, specular = { 0.f, 0.f, 0.f };
    float nDotL, rDotV;

    float3 absorbColor = { 0.f, 0.f, 0.f }, reflectColor = { 0.f, 0.f, 0.f }, transparencyColor = { 0.f, 0.f, 0.f };

    for (int lightIndex = 0; lightIndex < LIGHT_COUNT; ++lightIndex) {
        const Light* light = &lights[lightIndex];
        if (light->position.w != 0)
            lightVec = light->position.xyz - fPosition.xyz;
        else
            lightVec = -light->position.xyz;

        // Shoot ray towards light source, any hit means shadow.
        Ray rayToLight;
        rayToLight.start = (float4)(fPosition, 1.f);
        rayToLight.direction = (float4)(lightVec, 0.f);
        // Need 'skin' width to avoid hitting itself.
        rayToLight.start += 0.01f * (float4)(normalize(rayToLight.direction.xyz), 0);
        HitRecord shadowcastHit;
        shadowcastHit.time = MAX_FLOAT;

        raycast(OBJECT_COUNT, objs, &rayToLight, &shadowcastHit);

        lightVec = normalize(lightVec);

        float3 tNormal = fNormal;
        normalView = normalize(tNormal);
        nDotL = dot(normalView, lightVec);

        viewVec = -fPosition;
        viewVec = normalize(viewVec);

        reflectVec = reflect(-lightVec, normalView);
        reflectVec = normalize(reflectVec);

        rDotV = dot(reflectVec, viewVec);
        rDotV = fmax(rDotV, 0.0f);

        ambient = componentWiseMultiply(hit.mat.ambient, light->ambient);

        // Object cannot directly see the light
        if (shadowcastHit.time >= 1.f || shadowcastHit.time < 0) {
            diffuse = componentWiseMultiply(hit.mat.diffuse, light->diffuse) * fmax(nDotL, 0.f);
            if (nDotL > 0)
                specular = componentWiseMultiply(hit.mat.specular, light->specular) * pow(rDotV, fmax(hit.mat.shininess, 1.f));
        }
        else {
            diffuse = (float3)( 0., 0., 0. );
            specular = (float3)( 0., 0., 0. );
        }
        absorbColor = absorbColor + ambient + diffuse + specular;
    }

    fColor = absorbColor;

    pixelData[ii] = fColor;
}

