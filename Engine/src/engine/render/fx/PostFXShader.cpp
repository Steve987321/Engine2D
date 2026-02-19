#include "pch.h"
#include "EngineCore.h"
#include "engine/Logger.h"
#include "PostFXShader.h"

namespace Toad
{
    bool PostFXShader::Init()
    {
        if (!sf::Shader::isAvailable())
            return false;

        if (!LoadShader())
        {
            LOGERRORF("Failed to load post fx shader: '{}'", GetName());
            return false;
        }

        return true;
    }

    std::string_view PostFXShader::GetName() const 
    { 
        return "Unknown";
    }

	void PostFXShader::UpdateUniforms()
	{
	}

	void PostFXShader::ShowUI()
	{
	}

} // namespace Toad
