#include "pch.h"
#include "engine/Engine.h"
#include "Screen.h"

namespace Toad
{

namespace Screen
{
    static std::function<const ScreenDimensions&()> screen_content_info_provider;

    Vec2f ScreenToWorld(const Vec2i& point)
    {
        Camera* current_cam = Camera::GetActiveCamera();

        if (!current_cam) 
            return {-1, -1};

        return ScreenToWorld(point, *current_cam);
    }

    Vec2f ScreenToWorld(const Vec2i& point, const Camera& cam)
    {
        sf::RenderTexture& tex = GetActiveRenderTexture();
        float fx = cam.original_size.x / cam.GetSize().x;
        float fy = cam.original_size.y / cam.GetSize().y;
        Vec2i fixed_point = { (int)((float)point.x * fx), (int)(float)(point.y * fy) };
        return tex.mapPixelToCoords(fixed_point);
    }

    Vec2f WorldToScreen(const Vec2f &point, const Camera &cam) {
        assert(screen_content_info_provider && "Screen content info provider must be set before calling WorldToScreen");
        const ScreenDimensions& content_info = screen_content_info_provider();
        return WorldToScreen(point, cam, content_info.content_size, content_info.content_pos);
    }

    Vec2f WorldToScreen(const Vec2f& point, const Camera& cam, const Vec2f& content_size, const Vec2f& content_pos)
    {
        Vec2i obj_pos_px = GetActiveRenderTexture().mapCoordsToPixel(point, cam.GetView());
        Vec2f obj_pos_px_flt = { (float)obj_pos_px.x, (float)obj_pos_px.y };

        const Vec2f& cam_original_size = cam.original_size;
		float scale_x = content_size.x / cam_original_size.x;
		float scale_y = content_size.y / cam_original_size.y;

		obj_pos_px_flt.x *= scale_x;
		obj_pos_px_flt.y *= scale_y;
		obj_pos_px_flt.x += content_pos.x;
		obj_pos_px_flt.y += content_pos.y;

        return { obj_pos_px_flt.x, obj_pos_px_flt.y };
	}

    void SetScreenContentInfoProvider(std::function<const ScreenDimensions&()> content_info_provider) 
    {
        screen_content_info_provider = content_info_provider;
    }

}

}
