#pragma once

namespace filewatch
{
	enum class Event;
}

struct ImGuiContext;
namespace Toad
{

class Script;

using FENGINE_UI = std::function<void(ImGuiContext* ctx)>;
using FENGINE_PRE_UI_CALLBACK = std::function<void()>;
using FEVENT_CALLBACK = std::function<void(const sf::Event& ctx)>;
using FEDITOR_TEXTURE_DRAW_CALLBACK = std::function<void(sf::RenderTexture& texture)>;
using FONCLOSE_CALLBACK = std::function<void(int)>;
using FONDLLCHANGE_CALLBACK = std::function<void(const std::wstring&, const filewatch::Event)>;
using TGAME_SCRIPTS = std::unordered_map<std::string, Script*>;

using Vec2i = sf::Vector2i;
using Vec2f = sf::Vector2f;
using Vec2u = sf::Vector2u;
using Vec3i = sf::Vector3i;
using Vec3f = sf::Vector3f;
using Keyboard = sf::Keyboard;
using IntRect = sf::IntRect;
using Texture = sf::Texture;
using Color = sf::Color;

class ENGINE_API FloatRect : public sf::FloatRect
{
public:
	FloatRect()
		: sf::FloatRect()
	{}

	FloatRect(const sf::FloatRect& r)
	{
		width = r.width;
		height = r.height;
		left = r.left;
		top = r.top;
	}

public:
	void Expand(float v);
	bool Intersects(const FloatRect& other);
	bool Contains(const Vec2f& point);

private:
	using sf::FloatRect::intersects;
};

class ENGINE_API Mouse : public sf::Mouse
{
public: 
	static void SetVisible(bool visible);
	static const Vec2i& GetPosition();
	static void SetPosition(const Vec2i& pos);

	static inline bool mouse_visible = true;
	static inline bool capture_mouse = false;
	static inline Vec2i last_captured_mouse_pos {};
	static inline Vec2i relative_mouse_pos {};

private:
	using sf::Mouse::getPosition;
	using sf::Mouse::setPosition;
};

struct ENGINE_API AudioSource
{
	std::filesystem::path full_path;
	std::filesystem::path relative_path;
	sf::SoundBuffer sound_buffer;
	bool has_valid_buffer;
};


}
