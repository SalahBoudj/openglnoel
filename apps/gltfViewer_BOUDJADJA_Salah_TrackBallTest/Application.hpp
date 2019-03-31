#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/ViewController.hpp>
#include <glmlv/simple_geometry.hpp>
#include <glmlv/Image2DRGBA.hpp>
#include <map>
#include <tiny_gltf.h>
#include "Track.hpp"


class Application
{
public:
    Application(int argc, char** argv);

    int run();
private:

    //Pour les lights
    static glm::vec3 computeDirectionVector(float phiRadians, float thetaRadians)
    {
        const auto cosPhi = glm::cos(phiRadians);
        const auto sinPhi = glm::sin(phiRadians);
        const auto sinTheta = glm::sin(thetaRadians);
        return glm::vec3(sinPhi * sinTheta, glm::cos(thetaRadians), cosPhi * sinTheta);
    }

    //Taille fenêtre
    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ m_nWindowWidth, m_nWindowHeight, "Template" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

    //POUR LES PATHS
    const glmlv::fs::path m_AppPath;
    const std::string m_AppName;
    const std::string m_ImGuiIniFilename;
    const glmlv::fs::path m_ShadersRootPath;
    const glmlv::fs::path m_AssetsRootPath;

    //GLSL programs
    glmlv::GLProgram m_geometryPassProgram;
    glmlv::GLProgram m_shadingPassProgram;
    glmlv::GLProgram m_displayDepthProgram;
    glmlv::GLProgram m_displayPositionProgram;


    //Pour le format gltf
    std::map<std::string, int> attribNb;
    std::map<int, int> nbVec;

    std::vector<GLuint> vaos;
    std::vector<tinygltf::Primitive> primitives;
    std::vector<GLuint> textures;

    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    // Display depth pass uniforms
    GLint m_uGDepthSamplerLocation;

    // Display position pass uniforms
    GLint m_uGPositionSamplerLocation;
    GLint m_uSceneSizeLocation;
    glm::vec3 m_SceneSize = glm::vec3(1000.f); // Used for camera speed and projection matrix parameters
    float m_SceneSizeLength = 500.f;

    GLuint m_textureSampler = 0;

    // Triangle covering the whole screen, for the shading pass:
    GLuint m_TriangleVBO = 0;
    GLuint m_TriangleVAO = 0;

    // GBuffer:
    enum GBufferTextureType
    {
        GPosition = 0,
        GNormal,
        GDiffuse,
        GDepth,
        GBufferTextureCount
    };

    const char * m_GBufferTexNames[GBufferTextureCount + 1] = { "position", "normal", "diffuse", "depth","beauty" }; // Tricks, since we cant blit depth, we use its value to draw the result of the shading pass
    const GLenum m_GBufferTextureFormat[GBufferTextureCount] = {GL_RGB32F,GL_RGB32F, GL_RGB32F, GL_DEPTH_COMPONENT32F };
    GLuint m_GBufferTextures[GBufferTextureCount];
    GLuint m_GBufferFBO; // Framebuffer object

    GBufferTextureType m_CurrentlyDisplayed = GBufferTextureCount; // Default to beauty

    GLuint m_WhiteTexture; // A white 1x1 texture

    // Geometry pass uniforms
    GLint m_uModelViewProjMatrixLocation;
    GLint m_uModelViewMatrixLocation;
    GLint m_uNormalMatrixLocation;

    GLint m_uKdLocation;

    GLint m_uKdSamplerLocation;


    // Shading pass uniforms
    GLint m_uGBufferSamplerLocations[GDepth];
    GLint m_uDirectionalLightDirLocation;
    GLint m_uDirectionalLightIntensityLocation;
    GLint m_uPointLightPositionLocation;
    GLint m_uPointLightIntensityLocation;

    //Lights
    float m_DirLightPhiAngleDegrees = 322.f;
    float m_DirLightThetaAngleDegrees = 42.f;
    glm::vec3 m_DirLightDirection = computeDirectionVector(glm::radians(m_DirLightPhiAngleDegrees), glm::radians(m_DirLightThetaAngleDegrees));
    glm::vec3 m_DirLightColor = glm::vec3(1, 1, 1);
    float m_DirLightIntensity = 1.f;
    glm::vec3 m_PointLightPosition = glm::vec3(0, 1, 0);
    glm::vec3 m_PointLightColor = glm::vec3(1, 1, 1);
    float m_PointLightIntensity = 5.f;


    glmlv::ViewController m_viewController{ m_GLFWHandle.window(), 3.f };
};
