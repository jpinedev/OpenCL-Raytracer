#include "ObjectData.hpp"
#include <iostream>

ObjectData::ObjectData(PrimativeType type, Material& mat, glm::mat4 mv) :
    mat(mat),
    mv(mv),
    mvInverse(glm::inverse(mv)),
    mvInverseTranspose(glm::transpose(mvInverse)),
    type(type) { }


void ObjectData::Raycast(Ray3D ray, HitRecord& hit) const {
    ray.start = mvInverse * ray.start;
    ray.direction = mvInverse * ray.direction;

    switch (type) {
    case PrimativeType::sphere:
        RaycastSphere(ray, hit);
        break;
    case PrimativeType::box:
        RaycastBox(ray, hit);
        break;
    }
}

inline void ObjectData::RaycastSphere(Ray3D& ray, HitRecord& hit) const {
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
    if (radical < 0) return;

    float root = sqrtf(radical);

    float t1 = (-B - root) / (2.f * A);
    float t2 = (-B + root) / (2.f * A);

    float tMin = (t1 >= 0 && t2 >= 0) ? glm::min(t1, t2) : glm::max(t1, t2);
    // object is fully behind camera
    if (tMin < 0) return;

    // already hit a closer object
    if (hit.time <= tMin) return;

    hit.time = tMin;
    glm::vec4 objSpaceIntersection = ray.start + tMin * ray.direction;
    hit.intersection = mv * objSpaceIntersection;
    glm::vec3 objSpaceNormal(objSpaceIntersection);
    glm::vec4 normalDir = mvInverseTranspose * glm::vec4(objSpaceNormal, 0);
    glm::vec3 normal(normalDir);
    hit.normal = glm::normalize(normal);
    hit.mat = mat;
}

bool intersectsWidthBoxSide(float& tMin, float& tMax, float start, float dir) {
    float t1 = (-0.5f - start);
    float t2 = (0.5f - start);
    if (dir == 0) {
        // no intersection
        if (glm::sign(t1) == glm::sign(t2)) return false;

        tMin = -MAX_FLOAT;
        tMax = MAX_FLOAT;
        return true;
    }

    t1 /= dir;
    t2 /= dir;

    if (dir < 0) {
        tMin = glm::min(t1, t2);
        tMax = glm::max(t1, t2);
    }
    else {
        tMin = t1;
        tMax = t2;
    }

    return true;
}

inline void ObjectData::RaycastBox(Ray3D& ray, HitRecord& hit) const {
    float txMin, txMax, tyMin, tyMax, tzMin, tzMax;

    if (!intersectsWidthBoxSide(txMin, txMax, ray.start.x, ray.direction.x))
        return;

    if (!intersectsWidthBoxSide(tyMin, tyMax, ray.start.y, ray.direction.y))
        return;

    if (!intersectsWidthBoxSide(tzMin, tzMax, ray.start.z, ray.direction.z))
        return;

    float tMin = glm::max(glm::max(txMin, tyMin), tzMin);
    float tMax = glm::min(glm::min(txMax, tyMax), tzMax);

    // no intersection
    if (tMax < tMin) return;

    float tHit = (tMin >= 0 && tMax >= 0) ? glm::min(tMin, tMax) : glm::max(tMin, tMax);
    // object is fully behind camera
    if (tHit < 0) return;

    // already hit a closer object
    if (hit.time <= tHit) return;

    glm::vec4 objSpaceIntersection = ray.start + tHit * ray.direction;
    objSpaceIntersection.w = 1.f;

    glm::vec4 objSpaceNormal(0.f, 0.f, 0.f, 0.f);
    if (objSpaceIntersection.x > 0.4998f) objSpaceNormal.x += 1.f;
    else if (objSpaceIntersection.x < -0.4998f) objSpaceNormal.x -= 1.f;

    if (objSpaceIntersection.y > 0.4998f) objSpaceNormal.y += 1.f;
    else if (objSpaceIntersection.y < -0.4998f) objSpaceNormal.y -= 1.f;

    if (objSpaceIntersection.z > 0.4998f) objSpaceNormal.z += 1.f;
    else if (objSpaceIntersection.z < -0.4998f) objSpaceNormal.z -= 1.f;

    objSpaceNormal = glm::normalize(objSpaceNormal);

    hit.time = tHit;
    hit.intersection = mv * objSpaceIntersection; //ray.start + tHit * ray.direction;
    hit.normal = glm::normalize(glm::vec3(mvInverseTranspose * objSpaceNormal));
    hit.mat = mat;
}