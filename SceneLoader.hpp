#pragma once

#include <string>
#include <vector>
#include "ObjectData.hpp"
#include "Light.hpp"
#include <map>

class SceneLoader
{
public:
    void Load(const std::string& i_sceneFileLoc, std::vector<ObjectData>& o_objects, std::vector<Light>& o_lights);

private:
    void Init(const std::string& i_sceneFileLoc);

    void ParseHeader();

    void ParseBody(std::vector<ObjectData>& o_objects, std::vector<Light>& o_lights);

    bool GetNextLine(std::string& o_line, size_t& o_indent);

private:
    static const std::vector<std::string> HeaderCommands;
    static const std::string SectionDelimiter;
    static const std::vector<std::string> BodyCommands;

    // File loaded into memory
    std::vector<std::string> lines;

    // Material properties scraped from scene header
    std::map<std::string, Material> materials;
    // Light properties scraped from scene header
    std::map<std::string, LightProperties> lightProperties;

    // Parsing/formatting data
    size_t lineNum = 0;
    size_t lastIndent = 0;
};

