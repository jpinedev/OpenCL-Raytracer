#pragma once

#include <string>
#include <vector>
#include "ObjectData.hpp"
#include "Light.hpp"

class SceneLoader
{
public:
    SceneLoader() = delete;

    static void Load(const std::string& i_sceneFileLoc, std::vector<ObjectData>& o_objects, std::vector<Light>& o_lights);
    
};

