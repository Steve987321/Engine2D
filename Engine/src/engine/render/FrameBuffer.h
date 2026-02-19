#pragma once

namespace Toad
{

class FrameBuffer
{
public:
	~FrameBuffer();

    void Init(int w, int h);

	void Bind();
    void Unbind();
    void Resize(int w, int h);

    unsigned int GetTextureID() const;

private:
    unsigned int fbo_id;
    unsigned int texture_id;
    unsigned int rbo_id;

    int width;
    int height;
};

}
