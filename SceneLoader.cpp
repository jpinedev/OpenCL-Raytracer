#include "SceneLoader.hpp"
#include <stack>
#include <format>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

void SceneLoader::Load(const std::string& i_sceneFileLoc, std::vector<ObjectData>& o_objects, std::vector<Light>& o_lights)
{
    ifstream infile(i_sceneFileLoc);

    if (!infile.is_open()) {
        throw runtime_error(format("Scene file '{}' could not be found.", i_sceneFileLoc));
    }

    stack<glm::mat4> modelview;
    modelview.push(glm::mat4(1.f));

    // TODO: replace with camera setup
    modelview.top() *= glm::lookAt(glm::vec3(0, 0, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    modelview.push(modelview.top());

    size_t lineNum = 0;
    string line;
    stringstream stream;
    string command;
    glm::vec4 floats{ 0.f, 0.f, 0.f, 0.f };
    // TODO: add material parsing
    Material mat;

    size_t lastIndent = 0;

    while (getline(infile, line)) {
        ++lineNum;

        if (line.empty()) continue;

        size_t firstChar = line.find_first_not_of(' ');
        // Line is all whitespace
        if (firstChar == string::npos) continue;

        // Line is a comment
        if (line[firstChar] == '#') continue;

        if (firstChar % 2 != 0) {
            throw runtime_error(format("Error parsing scene file at line {}:\n\tline does not have proper indentation, must be multiples of two", lineNum));
        }
        if (firstChar > lastIndent) {
            throw runtime_error(format("Error parsing scene file at line {}:\n\tline is indented too far", lineNum));
        }

        while (lastIndent > firstChar) {
            lastIndent -= 2;
            modelview.pop();
        }

        stream = stringstream(line);

        stream >> command;

        if (command == "sphere") {
            // TODO: supply a material name to assign material
            o_objects.emplace_back(ObjectData::PrimativeType::sphere, mat, modelview.top());
        }
        else if (command == "box") {
            // TODO: supply a material name to assign material
            o_objects.emplace_back(ObjectData::PrimativeType::box, mat, modelview.top());
        }
        else if (command == "translate") {
            for (int ii = 0; ii < 3; ++ii) {
                if (!(stream >> floats[ii])) {
                    throw runtime_error(format("Error parsing scene file at line {}:\n\ttranslate expects 3 arguments, found {}\n\ttranslate <x> <y> <z>", lineNum, ii + 1));
                }
            }
            modelview.push(modelview.top());
            modelview.top() *= glm::translate(glm::mat4(1.f), glm::vec3(floats));

            // Indent future lines to apply this transformation
            lastIndent += 2;
        }
        else if (command == "scale") {
            for (int ii = 0; ii < 3; ++ii) {
                if (!(stream >> floats[ii])) {
                    throw runtime_error(format("Error parsing scene file at line {}:\n\tscale expects 3 arguments, found {}\n\tscale <x> <y> <z>", lineNum, ii + 1));
                }
            }
            modelview.push(modelview.top());
            modelview.top() *= glm::scale(glm::mat4(1.f), glm::vec3(floats));

            // Indent future lines to apply this transformation
            lastIndent += 2;
        }
        else if (command == "rotate") {
            for (int ii = 0; ii < 4; ++ii) {
                if (!(stream >> floats[ii])) {
                    throw runtime_error(format("Error parsing scene file at line {}:\n\trotate expects 4 arguments, found {}\n\trotate <angle in degrees> <axis x> <axis y> <axis z>", lineNum, ii + 1));
                }
            }
            modelview.push(modelview.top());
            modelview.top() *= glm::rotate(glm::mat4(1.f), glm::radians(floats.x), glm::normalize(glm::vec3(floats.y, floats.z, floats.w)));

            // Indent future lines to apply this transformation
            lastIndent += 2;
        }
    }
}
