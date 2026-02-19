#include "pch.h"

#ifdef _WIN32
#include "glad/glad.h"
#endif 

#include "FrameBuffer.h"
#include "HDRPipeline.h"

#include "engine/Logger.h"

#ifdef __APPLE__
#define GL_RGBA16F GL_RGBA16F_ARB
#define glGenFramebuffers glGenFramebuffersEXT
#endif 

namespace Toad
{

FrameBuffer::~FrameBuffer() 
{
    if (fbo_id)
        glDeleteFramebuffers(1, &fbo_id);
    if (texture_id)
        glDeleteTextures(1, &texture_id);
}

void FrameBuffer::Init(int w, int h)
{
    width = w;
    height = h;

    glGenFramebuffers(1, &fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glTexImage2D(
        GL_TEXTURE_2D,
        0, 
        GL_RGBA16F,
        width,
        height,
        0,
        GL_RGBA,
        GL_FLOAT,
        nullptr
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // prevent (bloom) bleeding near the edges 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER, 
        GL_COLOR_ATTACHMENT0, 
        GL_TEXTURE_2D, 
        texture_id, 
        0
    );

    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOGERROR("Failed to create framebuffer");

    Unbind();
}

void FrameBuffer::Bind() 
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

    glViewport(0, 0, width, height);
}

void FrameBuffer::Unbind() 
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::Resize(int w, int h)
{
    width = w;
    height = h;

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(
        GL_TEXTURE_2D,
        0, 
        GL_RGBA16F,
        width,
        height,
        0,
        GL_RGBA,
        GL_FLOAT,
        nullptr
    );
}

unsigned int FrameBuffer::GetTextureID() const
{
	return texture_id;
}
}