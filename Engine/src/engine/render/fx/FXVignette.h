#include "PostFXShader.h"

namespace Toad
{

class VignetteFX final : public PostFXShader
{
public:
    std::string_view GetName() const override;

    float strength = 0.8f;
    float radius = 0.8f;

protected:
    bool LoadShader() override;

    void UpdateUniforms() override;

    void ShowUI() override;
};

}
