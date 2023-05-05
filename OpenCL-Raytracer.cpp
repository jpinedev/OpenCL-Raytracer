#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include "Ray3D.hpp"
#include "ObjectData.hpp"
#include <chrono>
#include <vector>
#include <fstream>
#include <iostream>
#include "Light.hpp"
#include "SceneLoader.hpp"
#include "IRaytracer.hpp"
#include "OpenCLRaytracer.hpp"

#include "PPMExporter.hpp"
#include "OpenGLView.hpp"

Ray3D screenSpaceToViewSpace(float width, float height, glm::vec2 pos, float angle) {
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;
    float aspect = width / height;

    Ray3D out = Ray3D(glm::vec3(0, 0, 0), glm::vec3(pos.x - halfWidth, pos.y - halfHeight,
        -(halfHeight / tan(angle))));
    return out;
}

int main(int argc, char** argv) {
    // TODO: add flags for setting these vars
    //int width = 1920, height = 1080;
    int width = 2560, height = 1440;
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
        SceneLoader loader;
        loader.Load(sceneFileLoc, objects, lights);
    }
    catch (std::exception err) {
        std::cout << err.what() << std::endl;
        return 1;
    }

    std::cout << "Scene file loaded without any errors.\n";

    std::vector<Ray3D> rays;
    std::vector<HitRecord> rayHits(height * width);
    std::vector<float> pixelData(height * width * 3);
    
    for (int jj = 0; jj < height; ++jj) {
        for (int ii = 0; ii < width; ++ii) {
            rays.emplace_back(screenSpaceToViewSpace((float)width, (float)height, glm::vec2(ii, height - jj), fov));
        }
    }

    //IRaytracer* raytracer = (IRaytracer*)new CPURaytracer();
    IRaytracer* raytracer = (IRaytracer*)new OpenCLRaytracer(objects, lights, rays, 30);

    OpenGLView view;

    view.SetUpWindow(width, height);

    while (!view.ShouldWindowClose()) {
#if _DEBUG
        auto startTime = std::chrono::high_resolution_clock::now();
#endif

        auto pixelData = raytracer->Render();
        view.Display(pixelData);

#if _DEBUG
        auto endTime = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        std::cout << "Frame finished in " << duration.count() << "ms.\n";
#endif
    }


    view.TearDownWindow();

    //PPMExporter::ExportP3(outFileLoc, width, height, pixelData);

    return 0;
}