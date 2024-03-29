#include <vector>
#include <memory>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Mesh.h"
#include "Window.h"
#include "Camera.h"
#include "Shader.h"
#include "DirectionalLight.h"
#include "Material.h"
#include "Constans.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Texture.h"
#include "Model.h"
#include "ShadowMap.h"
#include "OmniShadowMap.h"
#include "SkyBox.h"

const float toRadians = 3.14159265f / 180.0f;

std::unique_ptr<Window> window;

std::vector<Mesh*> meshList;

std::vector<Shader*> shaderList;
Shader* directionalShadowShader;
Shader* omniShadowShader;

std::unique_ptr<Camera> camera;

std::unique_ptr<Texture> brickTexture;
std::unique_ptr<Texture> plainTexture;

std::unique_ptr<Material> shinyMaterial;
std::unique_ptr<Material> dullMaterial;

DirectionalLight* directionalLight;
UniformDirectionalLight* uniformDirectionalLight;

std::vector<PointLight> pointLights;
std::vector<UniformPointLight> uniformPointLight;

std::vector<SpotLight> spotLights;
std::vector<UniformSpotLight> uniformSpotLight;

std::vector<UniformOmniShadowMap> uniformOmniShadowMap;

std::unique_ptr<Model> xwing;
std::unique_ptr<Model> blackhack;

std::unique_ptr<SkyBox> skyBox;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

GLfloat blackHawkAngle = 0.0f;

GLuint uniformModel = 0, uniformProjection = 0, unifornmView = 0, uniformEyePosition = 0;
GLuint uniformSpecularIntesity = 0, uniformShininess = 0;

void calcAverageNormals(const std::vector<GLuint>& indices, std::vector<Shape>& vertices) {
    for ( size_t i = 0; i < indices.size(); i += 3 ) {
        unsigned int in0 = indices[i];
        unsigned int in1 = indices[i + 1];
        unsigned int in2 = indices[i + 2];

        glm::vec3 v1(vertices[in1].position.x - vertices[in0].position.x,
                     vertices[in1].position.y - vertices[in0].position.y,
                     vertices[in1].position.z - vertices[in0].position.z);

        glm::vec3 v2(vertices[in2].position.x - vertices[in0].position.x,
                     vertices[in2].position.y - vertices[in0].position.y,
                     vertices[in2].position.z - vertices[in0].position.z);

        glm::vec3 normal = glm::cross(v1, v2);
        normal = glm::normalize(normal);

        vertices[in0].normal.x += normal.x; vertices[in0].normal.y += normal.y; vertices[in0].normal.z += normal.z;
        vertices[in1].normal.x += normal.x; vertices[in1].normal.y += normal.y; vertices[in1].normal.z += normal.z;
        vertices[in2].normal.x += normal.x; vertices[in2].normal.y += normal.y; vertices[in2].normal.z += normal.z;
    }

    for (auto & vertice : vertices) {
        glm::vec3 vec(vertice.normal.x, vertice.normal.y, vertice.normal.z);
        vec = glm::normalize(vec);
        vertice.normal.x = vec.x; vertice.normal.y = vec.y; vertice.normal.z = vec.z;
    }
}

void createObjects() {
    std::vector<GLuint> indices {
            0, 3, 1,
            1, 3, 2,
            2, 3, 0,
            0, 1, 2
    };

    std::vector<Shape> vertices {
            { -1.0f, -1.0f, -0.6f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
            { 0.0f, -1.0f, 1.0f,	0.5f, 0.0f,	0.0f, 0.0f, 0.0f },
            { 1.0f, -1.0f, -0.6f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f }
    };

    std::vector<GLuint> floorIndices {
            0, 2, 1,
            1, 2, 3
    };

    std::vector<Shape> floorVertices {
            {-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f},
            {10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f},
            {-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f},
            {10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f}
    };

    calcAverageNormals(indices,vertices);

    Mesh* mesh = new Mesh();
    mesh->CreateMesh(vertices, indices);
    meshList.push_back(mesh);

    Mesh* mesh2 = new Mesh();
    mesh2->CreateMesh(floorVertices, floorIndices);
    meshList.push_back(mesh2);
}

void CreateShaders() {
    auto shader = new Shader();
    shader->CreateFormFiles( "Shaders/shader.vert", "Shaders/shader.frag");
    shaderList.push_back(shader);

    directionalShadowShader = new Shader();
    directionalShadowShader->CreateFormFiles("Shaders/directionalShadowMap.vert", "Shaders/directionalShadowMap.frag");

    omniShadowShader = new Shader();
    omniShadowShader->CreateFormFiles("Shaders/omniShadowMap.vert", "Shaders/omniShadowMap.geom", "Shaders/omniShadowMap.frag");
}

void RenderScene() {
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.5f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    brickTexture->UseTexture();
    shinyMaterial->UseMateril(uniformSpecularIntesity, uniformShininess);
    meshList[0]->RenderMesh();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    plainTexture->UseTexture();
    dullMaterial->UseMateril(uniformSpecularIntesity, uniformShininess);
    meshList[1]->RenderMesh();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-10.0f, 0.0f, 15.0f));
    model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    shinyMaterial->UseMateril(uniformSpecularIntesity, uniformShininess);
    xwing->RenderModel();

    blackHawkAngle += 0.1f;

    if ( blackHawkAngle > 360.f ) blackHawkAngle = 0.1f;

    model = glm::mat4(1.0f);
    model = glm::rotate(model, -blackHawkAngle * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-8.0f, 2.0f, 5.0f));
    model = glm::rotate(model, -20.0f * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::rotate(model, -90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    shinyMaterial->UseMateril(uniformSpecularIntesity, uniformShininess);
    blackhack->RenderModel();
}

void DirectionalShadowMapPass(DirectionalLight* _light) {
    directionalShadowShader->UseShader();

    glViewport(0, 0, _light->GetShadowMap()->GetShadowWidth(), _light->GetShadowMap()->GetShadowHeight());

    _light->GetShadowMap()->Write();
    glClear(GL_DEPTH_BUFFER_BIT);

    uniformModel = directionalShadowShader->GetModelLocation();

    ShadowMap::SetDirectionalLightTransform(_light->CalcLightTransform(), directionalShadowShader);

    directionalShadowShader->Validate();

    RenderScene();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OmniShadowMapPass(PointLight* _light) {
    omniShadowShader->UseShader();

    glViewport(0, 0, _light->GetShadowMap()->GetShadowWidth(), _light->GetShadowMap()->GetShadowHeight());

    _light->GetShadowMap()->Write();
    glClear(GL_DEPTH_BUFFER_BIT);

    uniformModel = omniShadowShader->GetModelLocation();

    glUniform3f(omniShadowShader->GetUniformLocation("lightPos"), _light->GetPosition().x, _light->GetPosition().y, _light->GetPosition().z);
    glUniform1f(omniShadowShader->GetUniformLocation("farPlane"), _light->GetFarPlane());
    OmniShadowMap::SetLightMatrices(omniShadowShader, _light->CalcLightTransform());

    omniShadowShader->Validate();

    RenderScene();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderPass(const glm::mat4& _projection, const glm::mat4 _viewMatrix) {
    glViewport(0, 0, 1366, 768);

    // glClearColor — specify clear values for the color buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // GL_COLOR_BUFFER_BIT - Indicates the buffers currently enabled for color writing.
    // GL_DEPTH_BUFFER_BIT - Indicates the depth buffer.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    skyBox->DrawSkyBox(_viewMatrix, _projection);

    shaderList[0]->UseShader();

    uniformModel = shaderList[0]->GetModelLocation();
    uniformProjection = shaderList[0]->GetProjectionLocation();
    unifornmView = shaderList[0]->GetViewLocation();
    uniformEyePosition = shaderList[0]->GetUniformLocation("eyePosition");
    uniformSpecularIntesity = shaderList[0]->GetUniformLocation("material.specularIntensity");
    uniformShininess = shaderList[0]->GetUniformLocation("material.shininess");

    glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(_projection));
    glUniformMatrix4fv(unifornmView, 1, GL_FALSE, glm::value_ptr(_viewMatrix));
    glUniform3f(uniformEyePosition, camera->getCameraPosition().x, camera->getCameraPosition().y, camera->getCameraPosition().z);

    DirectionalLight::SetDirectionalLight(*directionalLight, *uniformDirectionalLight);

//    PointLight::SetPointLights(pointLights, uniformPointLight, shaderList[0]->GetUniformLocation("pointLightCount"),
//            3, 0, uniformOmniShadowMap);

    SpotLight::SetPointLights(spotLights, uniformSpotLight, shaderList[0]->GetUniformLocation("spotLightCount"),
            3 + pointLights.size(), pointLights.size(), uniformOmniShadowMap);

    ShadowMap::SetDirectionalLightTransform(directionalLight->CalcLightTransform(),shaderList[0]);

    directionalLight->GetShadowMap()->Read(GL_TEXTURE2);
    ShadowMap::SetTexture(1, shaderList[0]);
    ShadowMap::SetDirectionalShadowMap(2, shaderList[0]);

    glm::vec3 lowerLight = camera->getCameraPosition();
    lowerLight.y -= 0.3f;
    spotLights[0].SetFlash(lowerLight, camera->getCameraDirection());

    shaderList[0]->Validate();

    RenderScene();
}

int main() {
    window = std::make_unique<Window>(1366, 768);
    window->Initialise();

    createObjects();
    CreateShaders();

    camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 5.0f, 0.5f);

    brickTexture = std::make_unique<Texture>("Textures/brick.png");
    brickTexture->LoadTextureA();

    plainTexture = std::make_unique<Texture>("Textures/dirt.png");
    plainTexture->LoadTextureA();

    shinyMaterial = std::make_unique<Material>(4.0f, 256);
    dullMaterial = std::make_unique<Material>(0.3f, 4);

    xwing = std::make_unique<Model>();
    xwing->LoadModel("Models/x-wing.obj");

    blackhack = std::make_unique<Model>();
    blackhack->LoadModel("Models/uh60.obj");

    directionalLight = new DirectionalLight(2048, 2048, glm::vec3(1.0f, 0.53f, 0.3f),
            0.1f, 0.9f, glm::vec3(-10.0f, -12.0f, 18.5f));

    uniformDirectionalLight = new UniformDirectionalLight();
    DirectionalLight::GetUDirectionalLight(*shaderList[0], *uniformDirectionalLight);

    pointLights = {
            { glm::vec2(1024, 1024), glm::vec2(0.1f, 100.0f), glm::vec3(0.0f, 0.0f, 1.0f), 0.0f, 1.0f,
              glm::vec3(5.0f, 2.0f, 0.0f), 0.3f, 0.1f, 0.1f },
            { glm::vec2(1024, 1024), glm::vec2(0.1f, 100.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, 1.0f,
              glm::vec3(-4.0f, 3.0f, 0.0f), 0.3f, 0.1f, 0.1f }
    };

    uniformPointLight = std::vector<UniformPointLight>(MAX_POINT_LIGHTS);
    PointLight::GetUPointLight(*shaderList[0], uniformPointLight);

    spotLights = {
            { glm::vec2(1024, 1024), glm::vec2(0.1f, 100.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.0f,
              2.0f, glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 0.0f, 0.0f, glm::vec3(0.0f, -1.0f, 0.0f), 20.0f },
            { glm::vec2(1024, 1024), glm::vec2(0.1f, 100.0f), glm::vec3(1.0f, 1.0f, 1.0f),0.0f,
              1.0f, glm::vec3(0.0f, -1.5f, 0.0f), 1.0f, 0.0f, 0.0f, glm::vec3(-100.0f, -1.0f, 0.0f), 20.0f }
    };

    uniformSpotLight = std::vector<UniformSpotLight>(MAX_POINT_LIGHTS);
    SpotLight::GetUPointLight(*shaderList[0], uniformSpotLight);

    uniformOmniShadowMap = std::vector<UniformOmniShadowMap>(MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS);
    OmniShadowMap::GetUniformsOmniShadowMap(uniformOmniShadowMap, shaderList[0]);

    skyBox = std::make_unique<SkyBox>( std::vector<std::string> {
        "Textures/Skybox/cupertin-lake_rt.tga",
        "Textures/Skybox/cupertin-lake_lf.tga",
        "Textures/Skybox/cupertin-lake_up.tga",
        "Textures/Skybox/cupertin-lake_dn.tga",
        "Textures/Skybox/cupertin-lake_bk.tga",
        "Textures/Skybox/cupertin-lake_ft.tga",
    } );

    glm::mat4 projection = glm::perspective(glm::radians(60.0f),
            static_cast<GLfloat>(window->GetBufferWidth()) / static_cast<GLfloat>(window->GetBufferHeight()),0.1f, 100.0f);

    // Loop until window closed
    while ( window->getShouldClose() ) {

        // This function returns the value of the GLFW timer.
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        lastTime = now;

        // This function processes only those events that are already in the event queue and then returns immediately.
        // Processing events will cause the window and input callbacks associated with those events to be called.
        glfwPollEvents();

        camera->KeyControl(window->getKeys(), deltaTime);
        camera->MouseControl(window->getXChange(), window->getYChange());

        if ( window->getKeys()[GLFW_KEY_L] ) {
            spotLights[0].Toggle();
            window->getKeys()[GLFW_KEY_L] = false;
        }

        DirectionalShadowMapPass(directionalLight);

        for ( auto& pointLight : pointLights ) {
            OmniShadowMapPass(&pointLight);
        }

        for ( auto& spotLight : spotLights ) {
            OmniShadowMapPass(&spotLight);
        }

        RenderPass(projection, camera->calculateViewMatrix());

        // glUseProgram — Installs a program object as part of current rendering state
        glUseProgram(0);

        window->SwapBuffers();
    }

    for ( auto& mesh : meshList ) {
        delete mesh;
    }

    for ( auto& shader : shaderList ) {
        delete shader;
    }

    delete directionalLight;
    delete uniformDirectionalLight;

    return 0;
}