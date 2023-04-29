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

    void Serialize(uint8_t*& dest) const;

    static const size_t SERIALIZED_SIZE;

    void Raycast(Ray3D ray, HitRecord& hit);

private:
    Material mat;
    glm::mat4 mv, mvInverse, mvInverseTranspose;
    PrimativeType type;

    inline void RaycastSphere(Ray3D& ray, HitRecord& hit);
    inline void RaycastBox(Ray3D& ray, HitRecord& hit);
};

