#include "OpenGLView.hpp"
#include <stdexcept>
#include <iostream>
#include <fstream>

OpenGLView::OpenGLView(OpenGLModel& model) : model(model)
{
}

void OpenGLView::Render()
{
    glfwPollEvents();

    glClear(GL_COLOR_BUFFER_BIT);

    // glDrawPixels(width, height, GL_RGBA, GL_FLOAT, pixelData);

    glUseProgram(shaderProgram);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

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

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    shaderProgram = glCreateProgram();
    GLuint vertShader = LoadShader(GL_VERTEX_SHADER, "vert_shader.glsl");
    GLuint fragShader = LoadShader(GL_FRAGMENT_SHADER, "shade.glsl");
    glAttachShader(shaderProgram, vertShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);

    GLint result;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &result);
    if (result == GL_FALSE) {
        GLint length;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &length);

        GLchar* infoLog = new GLchar[length + 1];
        glGetProgramInfoLog(shaderProgram, length, &length, infoLog);

        fprintf(stderr, "Unable to link shader program:\n\t%s\n", infoLog);
        delete[] infoLog;
    }
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    // Setup quad
    float vertices[] = {
        -1.f, -1.f, 0.f,
         1.f, -1.f, 0.f,
         1.f,  1.f, 0.f,
        -1.f,  1.f, 0.f,
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    LoadScene();
}

void OpenGLView::TearDownWindow()
{
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
}

bool OpenGLView::ShouldWindowClose()
{
    return glfwWindowShouldClose(window);
}

static std::string LoadShaderSourceFromFile(const std::string& sourceFile)
{
    std::string str;

    std::ifstream infile(sourceFile);
    if (!infile.is_open())
    {
        std::cerr << "Unable to open shader '" << sourceFile << "'" << std::endl;
        return str;
    }

    std::string line;
    while (getline(infile, line))
    {
        str += line + '\n';
    }
    return str;
}

GLuint OpenGLView::LoadShader(GLenum type, const std::string& sourceFile)
{
    GLuint shader = glCreateShader(type);

    const std::string source = LoadShaderSourceFromFile(sourceFile);
    const GLchar* sourceStr = source.c_str();
    glShaderSource(shader, 1, &sourceStr, NULL);
    glCompileShader(shader);

    GLint result;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

        GLchar* infoLog = new GLchar[length + 1];
        glGetShaderInfoLog(shader, length, &length, infoLog);

        fprintf(stderr, "Unable to compile shader '%s':\n%s\n", sourceFile.c_str(), infoLog);
        delete[] infoLog;
    }

    return shader;
}

void OpenGLView::LoadScene()
{
    glUseProgram(shaderProgram);
    glUniform2f(glGetUniformLocation(shaderProgram, "camera.frameSize"), float(width), float(height));
    glUniform1f(glGetUniformLocation(shaderProgram, "camera.fov"), 60.f);

    const GLuint MAX_OBJECTS = 16;
    const GLuint MAX_LIGHTS = 16;

    auto& objs = model.objs;
    const GLuint objCount = GLuint(std::min(size_t(MAX_OBJECTS), objs.size()));
    glUniform1ui(glGetUniformLocation(shaderProgram, "OBJECT_COUNT"), objCount);
    std::string prefix = "objs[";
    for (GLuint ii = 0; ii < objCount; ++ii)
    {
        auto& obj = objs[ii];
        std::string objPrefix = prefix + std::to_string(ii) + "].";
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, (objPrefix + "mv").c_str()), 1, GL_FALSE, glm::value_ptr(obj.mv));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, (objPrefix + "mvInverse").c_str()), 1, GL_FALSE, glm::value_ptr(obj.mvInverse));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, (objPrefix + "mvInverseTranspose").c_str()), 1, GL_FALSE, glm::value_ptr(obj.mvInverseTranspose));

        glUniform1ui(glGetUniformLocation(shaderProgram, (objPrefix + "type").c_str()), GLuint(obj.type));

        std::string matPrefix = objPrefix + "mat.";
        glUniform3fv(glGetUniformLocation(shaderProgram, (matPrefix + "ambient").c_str()), 1, glm::value_ptr(obj.mat.ambient));
        glUniform3fv(glGetUniformLocation(shaderProgram, (matPrefix + "diffuse").c_str()), 1, glm::value_ptr(obj.mat.diffuse));
        glUniform3fv(glGetUniformLocation(shaderProgram, (matPrefix + "specular").c_str()), 1, glm::value_ptr(obj.mat.specular));
        glUniform1f(glGetUniformLocation(shaderProgram, (matPrefix + "absorption").c_str()), obj.mat.absorption);
        glUniform1f(glGetUniformLocation(shaderProgram, (matPrefix + "reflection").c_str()), obj.mat.reflection);
        glUniform1f(glGetUniformLocation(shaderProgram, (matPrefix + "transparency").c_str()), obj.mat.transparency);
        glUniform1f(glGetUniformLocation(shaderProgram, (matPrefix + "shininess").c_str()), obj.mat.shininess);
    }

    auto& lights = model.lights;
    const GLuint lightCount = GLuint(std::min(size_t(MAX_LIGHTS), lights.size()));
    glUniform1ui(glGetUniformLocation(shaderProgram, "LIGHT_COUNT"), lightCount);
    prefix = "lights[";
    for (GLuint ii = 0; ii < lightCount; ++ii)
    {
        auto& light = lights[ii];
        std::string lightPrefix = prefix + std::to_string(ii) + "].";
        glUniform4fv(glGetUniformLocation(shaderProgram, (lightPrefix + "ambient").c_str()), 1, glm::value_ptr(light.ambient));
        glUniform3fv(glGetUniformLocation(shaderProgram, (lightPrefix + "diffuse").c_str()), 1, glm::value_ptr(light.diffuse));
        glUniform3fv(glGetUniformLocation(shaderProgram, (lightPrefix + "specular").c_str()), 1, glm::value_ptr(light.specular));
        glUniform4fv(glGetUniformLocation(shaderProgram, (lightPrefix + "position").c_str()), 1, glm::value_ptr(light.lightPosition));
    }
}
