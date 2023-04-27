#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Ray3D.hpp"
#include "ObjectData.hpp"
#include <chrono>
#include <vector>
#include <fstream>
#include <iostream>
#include "Light.hpp"
#include "SceneLoader.hpp"

Ray3D screenSpaceToViewSpace(float width, float height, glm::vec2 pos, float angle) {
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;
    float aspect = width / height;

    Ray3D out = Ray3D(glm::vec3(0, 0, 0), glm::vec3(pos.x - halfWidth, pos.y - halfHeight,
        -(halfHeight / tan(angle))));
    return out;
}

inline glm::vec3 shadeHitTest(HitRecord& hit) {
    return glm::vec3(1., 1., 1.);
}
inline glm::vec3 shadeNormals(HitRecord& hit) {
    return (hit.normal + glm::vec3(1., 1., 1.)) * 0.5f;
}
inline glm::vec3 shadeAmbient(HitRecord& hit) {
    return hit.mat.ambient;
}

int main(int argc, char** argv) {
    // TODO: add flags for setting these vars
    int width = 800, height = 800;
    float fov = glm::radians(60.f);
    fov *= 0.5f;
    // TODO: add flag for output file
    std::string outFileLoc = "render.ppm";

    if (argc > 2) {
        return 1;
    }

    std::string sceneFileLoc;
    if (argc < 2) {
        std::cout << "Enter the scene file to render:\n";
        std::cin >> sceneFileLoc;
    }
    else {
        sceneFileLoc = argv[1];
    }

    std::vector<ObjectData> objects;
    std::vector<Light> lights;

    try {
        SceneLoader::Load(sceneFileLoc, objects, lights);
    }
    catch (std::exception err) {
        std::cout << err.what() << std::endl;
        return 1;
    }

    std::cout << "Scene file loaded without any errors.\n";

    std::cout << "Rendering...\n";

    auto startTime = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<HitRecord> > rayHits(height);
    std::vector<std::vector<glm::vec3> > pixelData(height);

    for (int jj = 0; jj < height; ++jj) {
        rayHits[jj].resize(width);
        pixelData[jj].resize(width);
    }

    for (int jj = 0; jj < height; ++jj) {
        std::vector<HitRecord>& hitsRow = rayHits[jj];
        for (int ii = 0; ii < width; ++ii) {
            Ray3D ray = screenSpaceToViewSpace((float)width, (float)height, glm::vec2(ii, height - jj), fov);

            for (auto& obj : objects) {
                obj.Raycast(ray, hitsRow[ii]);
            }

            if (hitsRow[ii].time < MAX_FLOAT) {
                //pixelData[jj][ii] = shadeHitTest(hitsRow[ii]);
                pixelData[jj][ii] = shadeNormals(hitsRow[ii]);
                //pixelData[jj][ii] = shadeAmbient(hitsRow[ii]);
                //pixelData[jj][ii] = shade(hitsRow[ii], 30);
            }
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    std::cout << "Render finished in " << duration.count() << "ms.\n";

    std::cout << "Exporting to file '" << outFileLoc << "'...\n";

    std::ofstream op(outFileLoc);

    op << "P3" << "\n";
    op << width << " " << height << "\n";
    op << "255\n";
    for (int jj = 0; jj < height; ++jj) {
        for (int ii = 0; ii < width; ++ii) {
            pixelData[jj][ii] *= 255.f;
            op << glm::min(255, (int)floorf(pixelData[jj][ii].r)) << " ";
            op << glm::min(255, (int)floorf(pixelData[jj][ii].g)) << " ";
            op << glm::min(255, (int)floorf(pixelData[jj][ii].b)) << std::endl;
        }
    }

    std::cout << "Export finished.\n";

    return 0;
}