#include <iostream>
#include <cstring>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../lib/glm/glm.hpp"
#include "../lib/glm/gtc/matrix_transform.hpp"
#include "../lib/glm/gtc/type_ptr.hpp"

// Window dimensions
const GLint WIDTH = 800, HEIGHT = 600;

GLuint VAO, VBO, shader, uniformModel;

bool direction = true;
float triOffset  = 0.0f;
float triMaxoffset = 0.7f;
float triIncrement = 0.005f;

// Vertex Shader
static const char* vShader = "                                                \n\
#version 450                                                                  \n\
                                                                              \n\
layout (location = 0) in vec3 pos;											  \n\
uniform mat4 model;											                  \n\
                                                                              \n\
void main()                                                                   \n\
{                                                                             \n\
    gl_Position = model * vec4(0.4 * pos.x, 0.4 * pos.y, pos.z, 1.0);				  \n\
}";

// Fragment Shader
static const char* fShader = "                                                \n\
#version 450                                                                  \n\
                                                                              \n\
out vec4 colour;                                                              \n\
                                                                              \n\
void main()                                                                   \n\
{                                                                             \n\
    colour = vec4(0.0, 1.0, 0.0, 1.0);                                        \n\
}";


void createTriangle() {
    GLfloat vertices[] {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

void addShader(GLuint _program, const char* _shaderCode, GLenum _shaderType) {
    GLuint newShader = glCreateShader(_shaderType);

    const GLchar* shaderCode[1];
    shaderCode[0] = _shaderCode;

    GLint codeLenght[1];
    codeLenght[0] = strlen(_shaderCode);

    glShaderSource(newShader, 1, shaderCode, codeLenght);
    glCompileShader(newShader);

    GLint result = 0;
    GLchar eLog[1024] = { 0 };

    glGetShaderiv(newShader, GL_COMPILE_STATUS, &result);

    if ( !result ) {
        glGetShaderInfoLog(shader, sizeof(eLog), nullptr, eLog);
        std::cerr << "Error compiling " << _shaderType << " shader: " <<  eLog << std::endl;
        return ;
    }

    glAttachShader(_program, newShader);
}

void compileShader() {
    shader = glCreateProgram();

    if ( !shader ) {
        std::cerr << "Error creating shader program" << std::endl;
        return ;
    }

    addShader(shader, vShader, GL_VERTEX_SHADER);
    addShader(shader, fShader, GL_FRAGMENT_SHADER);

    GLint result = 0;
    GLchar eLog[1024] = { 0 };

    glLinkProgram(shader);
    glGetProgramiv(shader, GL_LINK_STATUS, &result);

    if ( !result ) {
        glGetProgramInfoLog(shader, sizeof(eLog), nullptr, eLog);
        std::cerr << "Error linking program: " << eLog << std::endl;
        return ;
    }

    glValidateProgram(shader);
    glGetProgramiv(shader, GL_VALIDATE_STATUS, &result);

    if ( !result ) {
        glGetProgramInfoLog(shader, sizeof(eLog), nullptr, eLog);
        std::cerr << "Error validating program: " << eLog << std::endl;
        return ;
    }

    uniformModel = glGetUniformLocation(shader, "model");
}

int main() {
    // Initialise GLFW
    if ( !glfwInit() ) {
        std::cout << "GLFW initialisation failed!" << std::endl;
        glfwTerminate();
        return 1;
    }

    // Setup GLFW window properties
    // OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Core profile = No backwards compatibility
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Allow forward compatibility
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Test Window", nullptr, nullptr);

    if ( !mainWindow ) {
        std::cout << "GLFW Window creation failed" << std::endl;
        glfwTerminate();
        return 1;
    }

    // Get buffer size information
    int bufferWidth, bufferHeight;
    glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

    // Set context for GLEW to use
    glfwMakeContextCurrent(mainWindow);

    // Allow modern extension features
    glewExperimental = GL_TRUE;

    if ( glewInit() != GLEW_OK ) {
        std::cout << "GLEW initialisation failed!" << std::endl;
        glfwDestroyWindow(mainWindow);
        glfwTerminate();
        return 1;
    }

    // Setup Viewport size
    glViewport(0, 0, bufferWidth, bufferHeight);

    createTriangle();
    compileShader();

    // Loop until window closed
    while ( !glfwWindowShouldClose(mainWindow) ) {
        // Get + Handle user input
        glfwPollEvents();

        if ( direction ) {
            triOffset += triIncrement;
        } else {
            triOffset -= triIncrement;
        }

        if ( std::abs(triOffset) >= triMaxoffset ) {
            direction = !direction;
        }

        // Clear Window
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader);

        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(triOffset, triOffset, 0.0f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glUseProgram(0);

        glfwSwapBuffers(mainWindow);
    }

    return 0;
}