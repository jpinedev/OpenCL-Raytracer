#include "SceneLoader.hpp"
#include <stack>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

void SceneLoader::Init(const std::string& sceneFileLoc) {
    // Clean out any data from previous loads
    lines.clear();

    std::map<std::string, Material> materials;
    std::map<std::string, LightProperties> lightProperties;

    // Load scene file into memory
    ifstream infile(sceneFileLoc);

    if (!infile.is_open()) {
        throw runtime_error(string_format("Scene file '%s' could not be found.", sceneFileLoc.c_str()));
    }

    string line;
    while (getline(infile, line)) {
        lines.push_back(line);
    }

    infile.close();
}

void SceneLoader::Load(const std::string& i_sceneFileLoc, std::vector<ObjectData>& o_objects, std::vector<Light>& o_lights)
{
    Init(i_sceneFileLoc);

    size_t lineNum = 0;
    stringstream stream;

    ParseHeader();
    ParseBody(o_objects, o_lights);
}

enum class HeaderParseItem {
    none,
    material,
    light,
    //camera
};

void SceneLoader::ParseHeader() {
    string line;
    size_t currentIndent;
    stringstream stream;
    string command;
    glm::vec4 floats{ 0.f, 0.f, 0.f, 0.f };

    HeaderParseItem parseState = HeaderParseItem::none;
    string propName;

    while (GetNextLine(line, currentIndent)) {
        if (line == "===") break;

        stream = stringstream(line);

        while (lastIndent > currentIndent) {
            lastIndent -= 2;
            parseState = HeaderParseItem::none;
        }

        stream = stringstream(line);

        stream >> command;

        switch (parseState) {
        case HeaderParseItem::none:
            if (command == "material") {
                parseState = HeaderParseItem::material;

                // Indent future lines to supply material properties
                lastIndent += 2;

                if (!(stream >> propName)) {
                    throw runtime_error(string_format("Error parsing scene file at line %d:\n\tmaterial expects 1 argument, found 0\n\tmaterial <material name>", lineNum));
                }
                materials.emplace(propName, Material());
            }
            else if (command == "light") {
                parseState = HeaderParseItem::light;

                // Indent future lines to supply light properties
                lastIndent += 2;

                if (!(stream >> propName)) {
                    throw runtime_error(string_format("Error parsing scene file at line %d:\n\tlight expects 1 argument, found 0\n\tlight <light name>", lineNum));
                }
                lightProperties.emplace(propName, LightProperties());
            }
            else {
                throw runtime_error(string_format("Error parsing scene file at line %d:\n\tunsupported command '%s' in header\n\tif you are trying to specify properties, ensure the correct level of indentation", lineNum, command.c_str()));
            }
            break;

        case HeaderParseItem::material:
            if (command == "ambient") {
                for (int ii = 0; ii < 3; ++ii) {
                    if (!(stream >> floats[ii])) {
                        throw runtime_error(string_format("Error parsing scene file at line %d:\n\tambient expects 3 arguments, found %d\n\tambient <r> <g> <b>", lineNum, ii + 1));
                    }
                }

                materials[propName].ambient = glm::vec3(floats);
            }
            else if (command == "diffuse") {
                for (int ii = 0; ii < 3; ++ii) {
                    if (!(stream >> floats[ii])) {
                        throw runtime_error(string_format("Error parsing scene file at line %d:\n\tdiffuse expects 3 arguments, found %d\n\tdiffuse <r> <g> <b>", lineNum, ii + 1));
                    }
                }

                materials[propName].diffuse = glm::vec3(floats);
            }
            else if (command == "specular") {
                for (int ii = 0; ii < 3; ++ii) {
                    if (!(stream >> floats[ii])) {
                        throw runtime_error(string_format("Error parsing scene file at line %d:\n\tspecular expects 3 arguments, found %d\n\tspecular <r> <g> <b>", lineNum, ii + 1));
                    }
                }

                materials[propName].specular = glm::vec3(floats);
            }
            else if (command == "absorption") {
                if (!(stream >> floats[0])) {
                    throw runtime_error(string_format("Error parsing scene file at line %d:\n\tabsorption expects 1 argument, found 0\n\tabsorption <absorption ratio>", lineNum));
                }

                materials[propName].absorption = floats[0];
            }
            else if (command == "reflection") {
                if (!(stream >> floats[0])) {
                    throw runtime_error(string_format("Error parsing scene file at line %d:\n\treflection expects 1 argument, found 0\n\treflection <reflection ratio>", lineNum));
                }

                materials[propName].reflection = floats[0];
            }
            else if (command == "transparency") {
                if (!(stream >> floats[0])) {
                    throw runtime_error(string_format("Error parsing scene file at line %d:\n\ttransparency expects 1 argument, found 0\n\ttransparency <transparency ratio>", lineNum));
                }

                materials[propName].transparency = floats[0];
            }
            else if (command == "shininess") {
                if (!(stream >> floats[0])) {
                    throw runtime_error(string_format("Error parsing scene file at line %d:\n\tshininess expects 1 argument, found 0\n\tshininess <shininess value>", lineNum));
                }

                materials[propName].shininess = floats[0];
            }
            else {
                if (command == "material" || command == "light") {
                    throw runtime_error(string_format("Error parsing scene file at line %d:\n\ttried to declare a %s in a nested scope, unindent to declare a new %s", lineNum, command.c_str(), command.c_str()));
                }

                throw runtime_error(string_format("Error parsing scene file at line %d:\n\tunsupported command '%s' while parsing material", lineNum, command.c_str()));
            }
            break;

        case HeaderParseItem::light:
            if (command == "ambient") {
                for (int ii = 0; ii < 3; ++ii) {
                    if (!(stream >> floats[ii])) {
                        throw runtime_error(string_format("Error parsing scene file at line %d:\n\tambient expects 3 arguments, found %d\n\tambient <r> <g> <b>", lineNum, ii + 1));
                    }
                }

                lightProperties[propName].ambient = glm::vec3(floats);
            }
            else if (command == "diffuse") {
                for (int ii = 0; ii < 3; ++ii) {
                    if (!(stream >> floats[ii])) {
                        throw runtime_error(string_format("Error parsing scene file at line %d:\n\tdiffuse expects 3 arguments, found %d\n\tdiffuse <r> <g> <b>", lineNum, ii + 1));
                    }
                }

                lightProperties[propName].diffuse = glm::vec3(floats);
            }
            else if (command == "specular") {
                for (int ii = 0; ii < 3; ++ii) {
                    if (!(stream >> floats[ii])) {
                        throw runtime_error(string_format("Error parsing scene file at line %d:\n\tspecular expects 3 arguments, found %d\n\tspecular <r> <g> <b>", lineNum, ii + 1));
                    }
                }

                lightProperties[propName].specular = glm::vec3(floats);
            }
            else {
                if (command == "material" || command == "light") {
                    throw runtime_error(string_format("Error parsing scene file at line %d:\n\ttried to declare a %s in a nested scope, unindent to declare a new %s", lineNum, command.c_str(), command.c_str()));
                }

                throw runtime_error(string_format("Error parsing scene file at line %d:\n\tunsupported command '%s' while parsing light", lineNum, command.c_str()));
            }
            break;
        }
    }

    // TODO: add validation step for defined materials
}

void SceneLoader::ParseBody(std::vector<ObjectData>& o_objects, std::vector<Light>& o_lights) {
    stack<glm::mat4> modelview;
    modelview.push(glm::mat4(1.f));

    // TODO: replace with camera setup
    modelview.top() *= glm::lookAt(glm::vec3(0, 0, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    modelview.push(modelview.top());

    string line;
    size_t currentIndent;
    stringstream stream;
    string command;
    glm::vec4 floats{ 0.f, 0.f, 0.f, 0.f };
    string primativeType, propName;

    while (GetNextLine(line, currentIndent)) {
        stream = stringstream(line);

        while (lastIndent > currentIndent) {
            lastIndent -= 2;
            modelview.pop();
        }

        stream = stringstream(line);

        stream >> command;

        // TODO: throw errors for TOO MANY arguments

        if (command == "primative") {
            if (!(stream >> primativeType)) {
                throw runtime_error(string_format("Error parsing scene file at line %d:\n\tprimative expects 2 argument, found 0\n\tprimative <primative type> <material name>", lineNum));
            }
            if (!(stream >> propName)) {
                throw runtime_error(string_format("Error parsing scene file at line %d:\n\tprimative expects 2 argument, found 1\n\tprimative <primative type> <material name>", lineNum));
            }

            ObjectData::PrimativeType type;
            if (primativeType == "sphere") type = ObjectData::PrimativeType::sphere;
            else if (primativeType == "box") type = ObjectData::PrimativeType::box;
            else {
                throw runtime_error(string_format("Error parsing scene file at line %d:\n\tunsupported primative type '%s'", lineNum, primativeType.c_str()));
            }

            o_objects.emplace_back(type, materials.at(propName), modelview.top());
        }
        else if (command == "light") {
            if (!(stream >> propName)) {
                throw runtime_error(string_format("Error parsing scene file at line %d:\n\tlight expects 1 argument, found 0\n\tlight <light name>", lineNum));
            }

            o_lights.emplace_back(lightProperties.at(propName), modelview.top());
        }
        else if (command == "translate") {
            for (int ii = 0; ii < 3; ++ii) {
                if (!(stream >> floats[ii])) {
                    throw runtime_error(string_format("Error parsing scene file at line %d:\n\ttranslate expects 3 arguments, found %d\n\ttranslate <x> <y> <z>", lineNum, ii + 1));
                }
            }
            modelview.push(modelview.top());
            modelview.top() *= glm::translate(glm::mat4(1.f), glm::vec3(floats));

            // Indent future lines to apply this transstring_formation
            lastIndent += 2;
        }
        else if (command == "scale") {
            for (int ii = 0; ii < 3; ++ii) {
                if (!(stream >> floats[ii])) {
                    throw runtime_error(string_format("Error parsing scene file at line %d:\n\tscale expects 3 arguments, found %d\n\tscale <x> <y> <z>", lineNum, ii + 1));
                }
            }
            modelview.push(modelview.top());
            modelview.top() *= glm::scale(glm::mat4(1.f), glm::vec3(floats));

            // Indent future lines to apply this transstring_formation
            lastIndent += 2;
        }
        else if (command == "rotate") {
            for (int ii = 0; ii < 4; ++ii) {
                if (!(stream >> floats[ii])) {
                    throw runtime_error(string_format("Error parsing scene file at line %d:\n\trotate expects 4 arguments, found %d\n\trotate <angle in degrees> <axis x> <axis y> <axis z>", lineNum, ii + 1));
                }
            }
            modelview.push(modelview.top());
            modelview.top() *= glm::rotate(glm::mat4(1.f), glm::radians(floats.x), glm::normalize(glm::vec3(floats.y, floats.z, floats.w)));

            // Indent future lines to apply this transstring_formation
            lastIndent += 2;
        }
        else {
            throw runtime_error(string_format("Error parsing scene file at line %d:\n\tunsupported command '%s' in body", lineNum, command.c_str()));
        }
    }
}

bool SceneLoader::GetNextLine(string& o_line, size_t& o_indent) {
    for (auto lineIt = lines.begin() + lineNum; lineIt != lines.end(); ++lineIt) {
        ++lineNum;

        if (lineIt->empty()) continue;

        size_t firstChar = lineIt->find_first_not_of(' ');
        // Line is all whitespace
        if (firstChar == string::npos) continue;

        // Line is a comment
        if (lineIt->at(firstChar) == '#') continue;

        if (firstChar % 2 != 0) {
            throw runtime_error(string_format("Error parsing scene file at line %d:\n\tline does not have proper indentation, must be multiples of two", lineNum));
        }
        if (firstChar > lastIndent) {
            throw runtime_error(string_format("Error parsing scene file at line %d:\n\tline is indented too far", lineNum));
        }

        o_line = *lineIt;
        o_indent = firstChar;
        return true;
    }

    return false;
}