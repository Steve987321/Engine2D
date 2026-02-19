
namespace Toad
{

inline std::string_view vertex_shader = R"(
#version 120

attribute vec2 aPos;
attribute vec2 aUV;

varying vec2 uv;

void main()
{
    uv = aUV;
    gl_Position = vec4(aPos,0.0,1.0);
}

)";

// pass-through
inline std::string_view blit_fragment_shader = R"(
#version 120

varying vec2 uv;
uniform sampler2D scene;

void main() 
{
    gl_FragColor = texture2D(scene, uv);
}
)";

inline std::string_view hdr_fragment_shader = R"(
#version 120
varying vec2 uv;
uniform sampler2D scene;
uniform sampler2D bloom_blur;
uniform float exposure;
uniform float bloom_intensity;

vec3 ACESFilm(vec3 x) 
{
    float a=2.51, b=0.03, c=2.43, d=0.59, e=0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

void main() 
{
    vec3 hdrColor = texture2D(scene, uv).rgb;
    vec3 bloomColor = texture2D(bloom_blur, uv).rgb;
    
    // Additive blending
    vec3 color = hdrColor + (bloomColor * bloom_intensity);

    color *= exposure;
    color = ACESFilm(color);
    color = pow(color, vec3(1.0/2.2)); // Gamma correction

    gl_FragColor = vec4(color, 1.0);
}
)";

inline std::string_view fragment_bright_shader = R"(
#version 120
varying vec2 uv;
uniform sampler2D scene;
uniform float bloom_threshold;

void main() 
{
    vec3 color = texture2D(scene, uv).rgb;
    // Calculate luminance
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > bloom_threshold) 
        gl_FragColor = vec4(color, 1.0);
    else
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
)";

inline std::string_view fragment_blur_shader = R"(
#version 120

varying vec2 uv;
uniform sampler2D image;
uniform bool horizontal;
uniform vec2 tex_offset;

void main() {             
    float weight[5];
    weight[0] = 0.227027;
    weight[1] = 0.1945946;
    weight[2] = 0.1216216;
    weight[3] = 0.054054;
    weight[4] = 0.016216;

    vec3 result = texture2D(image, uv).rgb * weight[0];
    
    if(horizontal) {
        for(int i = 1; i < 5; ++i) {
            result += texture2D(image, uv + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture2D(image, uv - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    } else {
        for(int i = 1; i < 5; ++i) {
            result += texture2D(image, uv + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture2D(image, uv - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    gl_FragColor = vec4(result, 1.0);
}
)";

}
