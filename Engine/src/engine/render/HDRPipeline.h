#pragma once 

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/glext.h>
#elif _WIN32
#include <GL/gl.h>
#include <GL/GLU.h>
#endif 

#include "EngineCore.h"
#include "engine/render/fx/PostFXShader.h"

#include "SFML/Graphics.hpp"

namespace Toad 
{

namespace HDRPipeline
{
    struct Settings
    {
        bool bloom = true;
        float bloom_threshold = 1.f;
        float bloom_intensity = 1.f;
        float bloom_spread = 1.f;
        uint32_t bloom_blur_passes = 8;
        float exposure = 1.f;
    };

    // setup framebuffer and shaders for HDR rendering
    void Init(int width, int height);

    void Render(sf::RenderTexture& scene_texture);

    void CleanUp();

    ENGINE_API void ApplySettings(const Settings& new_settings);
    ENGINE_API unsigned int GetFinalHDRTextureID();
    ENGINE_API const Settings& GetSettings();

    ENGINE_API void AddPostFXShader(PostFXShader* shader);

    ENGINE_API const std::vector<PostFXShader*>& GetPostFX();
}

}