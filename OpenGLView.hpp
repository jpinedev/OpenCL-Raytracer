#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <CL/cl.h>

#include <vector>
#include <string>
#include "OpenGLModel.h"

class OpenGLView
{
public:
    OpenGLView(OpenGLModel& model);

    void Render();

    void SetUpWindow(size_t width, size_t height);
    void TearDownWindow();

    bool ShouldWindowClose();

private:
    GLuint LoadShader(GLenum type, const std::string& source);

    void LoadScene();

    OpenGLModel& model;

    size_t width, height;
    GLFWwindow* window = NULL;

    GLuint shaderProgram;
    GLuint quadVAO, quadVBO;
    GLuint image;

    GLint cameraUniform;
};

