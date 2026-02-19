#include "pch.h"
#include "FXVignette.h"

#include "engine/render/shaders/HDRShadersEmbedded.h"

#include "imgui/imgui.h"

namespace Toad
{

static std::string_view vignette_shader = R"(
#version 120
varying vec2 uv;
uniform sampler2D scene;    // will be set by the pipeline

uniform float strength;     // 0.0 - 1.0 
uniform float radius;       // 0.0 - 1.0 

void main()
{
    vec2 centered = uv - vec2(0.5);

    float dist = length(centered);

    float v = smoothstep(radius, radius - strength, dist);
    
    vec4 color = texture2D(scene, uv);
    gl_FragColor = vec4(color.rgb * v, color.a);
}

)";

std::string_view VignetteFX::GetName() const 
{ 
    return "Vignette"; 
}

bool VignetteFX::LoadShader()
{
    return shader.loadFromMemory(vertex_shader, vignette_shader);
}

void VignetteFX::UpdateUniforms()
{
    shader.setUniform("strength", strength);
    shader.setUniform("radius", radius);
}

void VignetteFX::ShowUI()
{
    ImGui::Checkbox("Enabled", &is_enabled);
    ImGui::SliderFloat("Strength", &strength, 0.f, 1.f);
    ImGui::SliderFloat("Radius", &radius, 0.f, 1.f);
}

} // namespace Toad
