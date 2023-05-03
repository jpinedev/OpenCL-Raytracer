#pragma once

#include <memory>
#include <stdexcept>
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

    // Under CC0 1.0: From https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
    template<typename ... Args>
    std::string string_format(const std::string& format, Args ... args)
    {
        int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
        if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
        auto size = static_cast<size_t>(size_s);
        std::unique_ptr<char[]> buf(new char[size]);
        std::snprintf(buf.get(), size, format.c_str(), args ...);
        return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
    }

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

