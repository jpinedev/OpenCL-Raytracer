#pragma once

#include <vector>
#include <GLFW/glfw3.h>

#include <CL/cl.h>

class OpenGLView
{
public:
    OpenGLView();

    void Display(const cl_float4* pixelData);

    void SetUpWindow(size_t width, size_t height);
    void TearDownWindow();

    bool ShouldWindowClose();

private:
    size_t width, height;
    GLFWwindow* window = NULL;

    GLuint image;
};

