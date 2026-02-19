#pragma once

#include "EngineCore.h"

namespace Toad
{

class PostFXShader
{
public:
    ENGINE_API bool Init();

    ENGINE_API virtual std::string_view GetName() const; 

    ENGINE_API virtual void UpdateUniforms();

    ENGINE_API virtual void ShowUI();

    bool is_enabled = true;
    sf::Shader shader;

protected:
    virtual bool LoadShader() = 0;
};

};
