#include "OpenGLView.hpp"
#include <stdexcept>

OpenGLView::OpenGLView()
{
}

void OpenGLView::Display(const std::vector<float>& pixelData)
{
    glfwPollEvents();

    glClear(GL_COLOR_BUFFER_BIT);

    glDrawPixels(width, height, GL_RGB, GL_FLOAT, pixelData.data());

    glfwSwapBuffers(window);
}

void OpenGLView::SetUpWindow(size_t width, size_t height)
{
    if (!glfwInit())
        throw std::runtime_error("GLFW could not be initialized.");

    this->width = width;
    this->height = height;

    window = glfwCreateWindow(width, height, "OpenCL Render", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        throw std::runtime_error("GLFW window could not be created.");
    }

    glfwMakeContextCurrent(window);

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, 0.1, 1);
    glPixelZoom(1, -1);
    glRasterPos3f(0, height - 1, -0.3);
}

void OpenGLView::TearDownWindow()
{
    glfwTerminate();
}

bool OpenGLView::ShouldWindowClose()
{
    return glfwWindowShouldClose(window);
}
