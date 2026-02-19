#include "pch.h"

#ifdef _WIN32
#include "glad/glad.h"
#else 
#endif 

#include "HDRPipeline.h"

#include "shaders/HDRShadersEmbedded.h"

#include "engine/Logger.h"
#include "engine/Types.h"
#include "engine/Engine.h"

#include "FrameBuffer.h"

#ifdef __APPLE__
#define glBindVertexArray glBindVertexArrayAPPLE
#define glGenVertexArrays glGenVertexArraysAPPLE
#endif

namespace Toad
{

namespace HDRPipeline
{
static GLuint quadVAO;
static GLuint quadVBO;
static GLuint final_tex = 0;

static FrameBuffer fb_main{};
static FrameBuffer fb_bright{};
static FrameBuffer fb_blur[2]{};
static FrameBuffer fb_pingpong[2]{};

static Shader* aces_shader;
static Shader* bright_shader;
static Shader* blur_shader;
static Shader* blit_shader;

static Settings settings;

static std::vector<PostFXShader*> post_effects;

static void DrawQuad()
{
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Init(int width, int height)
{
#ifdef _WIN32
	gladLoadGLLoader((GLADloadproc) sf::Context::getFunction);
#endif

	aces_shader = new Shader();
	bright_shader = new Shader();
	blur_shader = new Shader();
	blit_shader = new Shader();

	LOGDEBUGF("OpenGL version: {}", (const char*) glGetString(GL_VERSION));
	LOGDEBUGF("GLSL version: {}", (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION));

	fb_main.Init(width, height);
	fb_bright.Init(width, height);
	fb_blur[0].Init(width, height);
	fb_blur[1].Init(width, height);
	fb_pingpong[0].Init(width, height);
	fb_pingpong[1].Init(width, height);

	if (!aces_shader->loadFromMemory(vertex_shader, hdr_fragment_shader))
	{
		LOGERROR("Failed to load HDR shader");
		return;
	}
	if (!bright_shader->loadFromMemory(vertex_shader, fragment_bright_shader))
	{
		LOGERROR("Failed to load bright pass shader");
		return;
	}
	if (!blur_shader->loadFromMemory(vertex_shader, fragment_blur_shader))
	{
		LOGERROR("Failed to load blur shader");
		return;
	}
	if (!blit_shader->loadFromMemory(vertex_shader, blit_fragment_shader))
	{
		LOGERROR("Failed to load blit shader");
		return;
	}

    float quad[] =
    {
        -1,-1, 0,0,
        1,-1, 1,0,
        1, 1, 1,1,

        -1,-1, 0,0,
        1, 1, 1,1,
        -1, 1, 0,1
    };

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);

	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) (2 * sizeof(float)));
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Render(sf::RenderTexture& scene_texture)
{
	glDisable(GL_FRAMEBUFFER_SRGB);
	// get bright areas
	fb_bright.Bind();
	glClear(GL_COLOR_BUFFER_BIT);
	Shader::bind(bright_shader);
	bright_shader->setUniform("bloom_threshold", settings.bloom_threshold);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scene_texture.getTexture().getNativeHandle());
	bright_shader->setUniform("scene", 0);
	DrawQuad();
	fb_bright.Unbind();

	// guassian blur the bright areas
	bool horizontal = true;
	bool first_iteration = true;
	Shader::bind(blur_shader);

	blur_shader->setUniform("tex_offset", Vec2f{1.f / scene_texture.getSize().x * settings.bloom_spread, 1.f / scene_texture.getSize().y * settings.bloom_spread});

	for (int i = 0; i < settings.bloom_blur_passes; ++i)
	{
		fb_blur[horizontal].Bind();
		glClear(GL_COLOR_BUFFER_BIT);
		blur_shader->setUniform("horizontal", horizontal);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, first_iteration ? fb_bright.GetTextureID() : fb_blur[!horizontal].GetTextureID());
		blur_shader->setUniform("image", 0);
		DrawQuad();

		horizontal = !horizontal;
		first_iteration = false;
	}

	fb_blur[0].Unbind();
	glViewport(0, 0, scene_texture.getSize().x, scene_texture.getSize().y);

	fb_main.Bind();
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	Shader::bind(aces_shader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scene_texture.getTexture().getNativeHandle());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fb_blur[!horizontal].GetTextureID());

	aces_shader->setUniform("scene", 0);
	aces_shader->setUniform("bloom_blur", 1);
	aces_shader->setUniform("bloom_intensity", settings.bloom_intensity);
	aces_shader->setUniform("exposure", settings.exposure);

	DrawQuad();

	// post processing effects

	bool current_fb = 0;
	final_tex = fb_main.GetTextureID();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	for (PostFXShader* effect : post_effects)
	{
		if (!effect->is_enabled)
			continue;

		fb_pingpong[current_fb].Bind();
		glClear(GL_COLOR_BUFFER_BIT);

		Shader::bind(&effect->shader);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, final_tex);

		effect->shader.setUniform("scene", 0);
		effect->UpdateUniforms();

		DrawQuad();
		fb_pingpong[current_fb].Unbind();

		// the output of this pass becomes the input for the next
		final_tex = fb_pingpong[current_fb].GetTextureID();
		current_fb = !current_fb; // bing bong
	}

	// Shader::bind(blit_shader);

	////glBindFramebuffer(GL_FRAMEBUFFER, fb_pingpong[current_fb])
	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, final_tex);
	////glEnable(GL_FRAMEBUFFER_SRGB); // enable gamma correction for final output gives white filter over everything including ui

	// blit_shader->setUniform("scene", 0);

	// DrawQuad();

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	Shader::bind(nullptr);
	GetWindow().resetGLStates();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, GetWindow().getSize().x, GetWindow().getSize().y);
}

void CleanUp()
{
}

void ApplySettings(const Settings& new_settings)
{
	settings = new_settings;
}

unsigned int GetFinalHDRTextureID()
{
	return final_tex;
}

const Settings& GetSettings()
{
	return settings;
}

void AddPostFXShader(PostFXShader* shader)
{
	post_effects.push_back(shader);
}

const std::vector<PostFXShader*>& GetPostFX()
{
	return post_effects;
}

} // namespace HDRPipeline
} // namespace Toad
