#include "CPURaytracer.hpp"

inline glm::vec3 shadeHitTest(HitRecord& hit) {
    return glm::vec3(1., 1., 1.);
}
inline glm::vec3 shadeNormals(HitRecord& hit) {
    return (hit.normal + glm::vec3(1., 1., 1.)) * 0.5f;
}
inline glm::vec3 shadeAmbient(HitRecord& hit) {
    return hit.mat.ambient;
}

bool raycast(const std::vector<ObjectData>& objects, const Ray3D& ray, HitRecord& hit)
{
    for (auto& obj : objects) {
        obj.Raycast(ray, hit);
    }
    if (hit.time == MAX_FLOAT) return false;

    hit.reflection = glm::reflect(glm::vec3(ray.direction), hit.normal);
    return true;
}

int CPURaytracer::HitTest(const std::vector<ObjectData>& objects, const std::vector<Ray3D>& rays, std::vector<HitRecord>& hits, std::vector<float>& pixelData)
{
    for (size_t ii = 0; ii < rays.size(); ++ii) {
        if (raycast(objects, rays[ii], hits[ii])) {
            glm::vec3 color = shadeHitTest(hits[ii]);
            memcpy(&pixelData[ii * 3], glm::value_ptr(color), sizeof(float) * 3);
        }
    }
    return 0;
}

inline glm::vec3 componentWiseMultiply(const glm::vec3& lhs, const glm::vec3& rhs)
{
    return glm::vec3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
}

glm::vec3 shade(const std::vector<ObjectData>& objects, const std::vector<Light>& lights, const HitRecord& hit, const int MAX_BOUNCES = 0)
{
    const glm::vec3& fPosition = hit.intersection;
    const glm::vec3& fNormal = hit.normal;
    glm::vec3 fColor(0.f, 0.f, 0.f);
    glm::vec3 lightVec(0.f, 0.f, 0.f), viewVec(0.f, 0.f, 0.f), reflectVec(0.f, 0.f, 0.f);
    glm::vec3 normalView(0.f, 0.f, 0.f);
    glm::vec3 ambient(0.f, 0.f, 0.f), diffuse(0.f, 0.f, 0.f), specular(0.f, 0.f, 0.f);
    float nDotL, rDotV;

    glm::vec3 absorbColor(0.f, 0.f, 0.f), reflectColor(0.f, 0.f, 0.f), transparencyColor(0.f, 0.f, 0.f);

    for (auto& light : lights) {
        if (light.lightPosition.w != 0)
            lightVec = glm::vec3(light.lightPosition) - fPosition;
        else
            lightVec = -glm::vec3(light.lightPosition);

        // Shoot ray towards light source, any hit means shadow.
        Ray3D rayToLight(fPosition, lightVec);
        // Need 'skin' width to avoid hitting itself.
        rayToLight.start += glm::vec4(0.01f * glm::normalize(glm::vec3(rayToLight.direction)), 0);
        HitRecord shadowcastHit;

        raycast(objects, rayToLight, shadowcastHit);

        lightVec = glm::normalize(lightVec);

        glm::vec3 tNormal = fNormal;
        normalView = glm::normalize(tNormal);
        nDotL = glm::dot(normalView, lightVec);

        viewVec = -fPosition;
        viewVec = glm::normalize(viewVec);

        reflectVec = glm::reflect(-lightVec, normalView);
        reflectVec = glm::normalize(reflectVec);

        rDotV = glm::dot(reflectVec, viewVec);
        rDotV = glm::max(rDotV, 0.0f);

        ambient = componentWiseMultiply(hit.mat.ambient, light.ambient);
        // Object cannot directly see the light
        if (shadowcastHit.time >= 1.f || shadowcastHit.time < 0) {
            diffuse = componentWiseMultiply(hit.mat.diffuse, light.diffuse) * glm::max(nDotL, 0.f);
            if (nDotL > 0)
                specular = componentWiseMultiply(hit.mat.specular, light.specular) *
                glm::pow(rDotV, glm::max(hit.mat.shininess, 1.f));
        }
        else {
            diffuse = { 0., 0., 0. };
            specular = { 0., 0., 0. };
        }
        absorbColor = absorbColor + ambient + diffuse + specular;
    }

    if (MAX_BOUNCES > 0 && hit.mat.absorption <= 0.999f) {
        Ray3D reflectionRay(fPosition, hit.reflection);
        reflectionRay.start += glm::normalize(reflectionRay.direction) * 0.001f;
        HitRecord reflectionHit;

        if (raycast(objects, reflectionRay, reflectionHit))
            reflectColor = shade(objects, lights, reflectionHit, MAX_BOUNCES - 1);

        fColor = absorbColor * hit.mat.absorption
            + reflectColor * hit.mat.reflection
            + transparencyColor * hit.mat.transparency;
    }
    else {
        fColor = absorbColor;
    }

    return fColor;
}

int CPURaytracer::Shade(const std::vector<ObjectData>& objects, const std::vector<Light>& lights, const std::vector<Ray3D>& rays, std::vector<HitRecord>& hits, std::vector<float>& pixelData)
{
    ShadeWithReflections(0, objects, lights, rays, hits, pixelData);
    return 0;
}
#include <iostream>
#include <chrono>

int CPURaytracer::ShadeWithReflections(const unsigned int MAX_BOUNCES, const std::vector<ObjectData>& objects, const std::vector<Light>& lights, const std::vector<Ray3D>& rays, std::vector<HitRecord>& hits, std::vector<float>& pixelData)
{

    std::cout << "Executing...\n";

    auto startTime = std::chrono::high_resolution_clock::now();

    for (int ii = 0; ii < rays.size(); ++ii) {
        if (raycast(objects, rays[ii], hits[ii])) {
            glm::vec3 color = shade(objects, lights, hits[ii], MAX_BOUNCES);
            memcpy(&pixelData[ii * 3], glm::value_ptr(color), sizeof(float) * 3);
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    std::cout << "Calculating finished in " << duration.count() << "ms.\n";
    return 0;
}
