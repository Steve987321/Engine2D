#pragma once

#include <filesystem>
#include <functional>

namespace filewatch
{
	enum class Event;
}

namespace sf
{
	class Event;
	class RenderTexture;
}

struct ImGuiContext;
namespace Toad
{

class Script;

#ifdef _WIN32
using TFILEWATCH_STRTYPE = std::wstring;
#else
using TFILEWATCH_STRTYPE = std::string;
#endif

using FENGINE_UI = std::function<void(ImGuiContext* ctx)>;
using FENGINE_PRE_UI_CALLBACK = std::function<void()>;
using FEVENT_CALLBACK = std::function<void(const sf::Event& ctx)>;
using FEDITOR_TEXTURE_DRAW_CALLBACK = std::function<void(sf::RenderTexture& texture)>;
using FONCLOSE_CALLBACK = std::function<void(int)>;
using FONDLLCHANGE_CALLBACK = std::function<void(const TFILEWATCH_STRTYPE&, const filewatch::Event)>;
using TGAME_SCRIPTS = std::unordered_map<std::string, Script*>;

using Vec2i = sf::Vector2i;
using Vec2u = sf::Vector2u;
using Vec3i = sf::Vector3i;
using Vec3f = sf::Vector3f;
using Keyboard = sf::Keyboard;
using IntRect = sf::IntRect;
using Texture = sf::Texture;
using Color = sf::Color;

class ENGINE_API Vec2f : public sf::Vector2f
{
public:
    Vec2f();
    Vec2f(float x, float y);
    Vec2f(const Vec2f& v);
    Vec2f(const sf::Vector2f& other);
    
    void operator=(const sf::Vector2f& other);
    
    Vec2f operator+(const Vec2f& other) const;
    Vec2f operator-(const Vec2f& other) const;
    Vec2f operator*(const Vec2f& other) const;
    Vec2f operator/(const Vec2f& other) const;
    Vec2f operator+(float scalar) const;
    Vec2f operator*(float scalar) const;
    Vec2f operator/(float scalar) const;

    void operator+=(const Vec2f& other);
    void operator-=(const Vec2f& other);
    void operator-=(float scalar);
    void operator+=(float scalar);
    void operator*=(float scalar);
    void operator/=(float scalar);
    
    float Length() const;
    Vec2f Normalize() const;
    float Cross(const Vec2f& v) const;
    float Dot(const Vec2f& v) const;
};

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
	static Vec2i GetPosition();
    static Vec2i GetPositionRaw();
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

template <typename T>
inline Vec2f operator*(T left, const Vec2f& right)
{
    return Vec2f(right.x * left, right.y * left);
}

// put somewhere else? 
inline std::ostream& operator<<(std::ostream& o, const Toad::Vec2f& v)
{
    o << '(' << v.x << ',' << v.y << ')';
    return o;
}

}
