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
namespace Keyboard = sf::Keyboard;
using IntRect = sf::IntRect;
using Texture = sf::Texture;
using Color = sf::Color;

class Vec2f : public sf::Vector2f
{
public:
    ENGINE_API Vec2f();
    ENGINE_API Vec2f(float x, float y);
    ENGINE_API Vec2f(const Vec2f& v);
    ENGINE_API Vec2f(const sf::Vector2f& other);
    
    ENGINE_API void operator=(const sf::Vector2f& other);
    
    ENGINE_API Vec2f operator+(const Vec2f& other) const;
    ENGINE_API Vec2f operator-(const Vec2f& other) const;
    ENGINE_API Vec2f operator*(const Vec2f& other) const;
    ENGINE_API Vec2f operator/(const Vec2f& other) const;
    ENGINE_API Vec2f operator+(float scalar) const;
    ENGINE_API Vec2f operator*(float scalar) const;
    ENGINE_API Vec2f operator/(float scalar) const;

    ENGINE_API void operator+=(const Vec2f& other);
    ENGINE_API void operator-=(const Vec2f& other);
    ENGINE_API void operator-=(float scalar);
    ENGINE_API void operator+=(float scalar);
    ENGINE_API void operator*=(float scalar);
    ENGINE_API void operator/=(float scalar);
    
    ENGINE_API float Length() const;
	ENGINE_API float LengthSquared() const;
    ENGINE_API Vec2f Normalize() const;
    ENGINE_API float Cross(const Vec2f& v) const;
    ENGINE_API float Dot(const Vec2f& v) const;
};

class ENGINE_API FloatRect : public sf::FloatRect
{
public:
	FloatRect()
		: sf::FloatRect()
	{
	}

	FloatRect(const sf::FloatRect& r)
	{
		*this = r;
	}

public:
	void Expand(float v);
	bool Intersects(const FloatRect& other);
	bool Contains(const Vec2f& point);

private:
	using sf::FloatRect::contains;
};

namespace Mouse
{
	ENGINE_API void SetVisible(bool visible);
	ENGINE_API Vec2i GetPosition();
	ENGINE_API Vec2i GetPositionRaw();
	ENGINE_API void SetPosition(const Vec2i& pos);

    // Used to set the mouse relative to the current interacting screen
    ENGINE_API void SetRelativeMousePosition(const Vec2i& pos);
    ENGINE_API const Vec2i& GetRelativeMousePosition();

    ENGINE_API void ShouldCaptureMouse(bool capture);
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
    if constexpr (std::is_same<T, sf::Vector2<float>>::value)
    {
        return Vec2f(right.x * left.x, right.y * left.y);
    }
    else 
    {
        return Vec2f(right.x * left, right.y * left);
    } 
}

// put somewhere else? 
inline std::ostream& operator<<(std::ostream& o, const Toad::Vec2f& v)
{
    o << '(' << v.x << ',' << v.y << ')';
    return o;
}

}
