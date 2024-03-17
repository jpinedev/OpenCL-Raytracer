#include "OpenGLModel.h"

inline void StoreVec3(const glm::vec3& vec, float out[3])
{
    out[0] = vec.x;
    out[1] = vec.y;
    out[2] = vec.z;
}
inline void StoreVec4(const glm::vec4& vec, float out[4])
{
    out[0] = vec.x;
    out[1] = vec.y;
    out[2] = vec.z;
    out[3] = vec.w;
}

GLMaterial::GLMaterial(const Material& mat) : absorption(mat.absorption), reflection(mat.reflection), transparency(mat.transparency), shininess(mat.shininess)
{
    StoreVec3(mat.ambient, ambient);
    StoreVec3(mat.diffuse, diffuse);
    StoreVec3(mat.specular, specular);
}

inline void StoreMat4(const glm::mat4& mat, float out[16])
{
    for (GLuint ii = 0; ii < 4; ++ii)
    {
        StoreVec4(mat[ii], out + ii * 4);
    }
}

GLObjectData::GLObjectData(const ObjectData& obj) : mat(obj.mat), type((uint8_t)obj.type)
{
    StoreMat4(obj.mv, mv);
    StoreMat4(obj.mvInverse, mvInverse);
    StoreMat4(obj.mvInverseTranspose, mvInvTranspose);
}

GLLight::GLLight(const Light& light)
{
    StoreVec4(light.lightPosition, position);
    StoreVec3(light.ambient, ambient);
    StoreVec3(light.diffuse, diffuse);
    StoreVec3(light.specular, specular);
}

OpenGLModel::OpenGLModel(const float maxBounces, const std::vector<ObjectData> objs, const std::vector<Light> lights) : MAX_BOUNCES(maxBounces), objs(objs), lights(lights)
{
}
