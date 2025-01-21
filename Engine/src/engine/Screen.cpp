#include "pch.h"
#include "engine/Engine.h"
#include "Screen.h"

namespace Toad
{

namespace Screen
{
    Vec2f ScreenToWorld(const Vec2i& point)
    {
        sf::RenderTexture& tex = Engine::Get().GetActiveRenderTexture();

        Camera* current_cam = Engine::Get().interacting_camera;
        if (!current_cam) 
            return {-1, -1};

        return ScreenToWorld(point, *current_cam);
    }

    Vec2f ScreenToWorld(const Vec2i& point, const Camera& cam)
    {
        sf::RenderTexture& tex = Engine::Get().GetActiveRenderTexture();
        float fx = cam.original_size.x / cam.GetSize().x;
        float fy = cam.original_size.y / cam.GetSize().y;
        Vec2i fixed_point = { (int)((float)point.x * fx), (int)(float)(point.y * fy) };
        return tex.mapPixelToCoords(fixed_point);
    }

    Vec2f WorldToScreen(const Vec2f& point, const Camera& cam, const Vec2f& content_size, const Vec2f& content_pos)
    {
        Vec2i obj_pos_px = Engine::Get().GetActiveRenderTexture().mapCoordsToPixel(point, cam.GetView());
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

}

}
