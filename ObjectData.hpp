#pragma once
#include "HitRecord.hpp"
#include "Material.hpp"
#include "Ray3D.hpp"

class ObjectData
{
public:
    enum class PrimativeType : uint8_t {
        sphere,
        box
    };

public:
    ObjectData(PrimativeType type, Material& mat, glm::mat4 mv);

    // void Serialize(void* dest);
    // static Material Deserialize(void* src);

    void Raycast(Ray3D ray, HitRecord& hit);

private:
    Material mat;
    glm::mat4 mv, mvInverse, mvInverseTranspose;
    PrimativeType type;

    inline void RaycastSphere(Ray3D& ray, HitRecord& hit);
    inline void RaycastBox(Ray3D& ray, HitRecord& hit);
};

