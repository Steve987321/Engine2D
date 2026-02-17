#pragma once

#include "Object.h"

namespace Toad
{

class ENGINE_API Camera : public Object
{
public:
	explicit Camera(std::string_view obj_name);
	~Camera();

	Vec2f original_size{0,0};

	json Serialize() override;

public:
    // returns the active camera in the scene or interacting camera when in the editor 
    // scripts should prefer using this instead of GetActiveSceneCamera() to get the active camera
	static Camera* GetActiveCamera();

    // returns tha active camera in the scene 
    static Camera* GetActiveSceneCamera();

public:
	void OnCreate() override;
	void Start() override;
	void Render(sf::RenderTarget& target) override;
	void Update() override;

    const Vec2f& GetPosition() const override;
	void SetPosition(const Vec2f& position) override;

	float GetRotation() override;
	void SetRotation(float degrees) override;

	bool IsActive() const;

	void ActivateCamera();
	void DeactivateCamera();

	void SetSize(const Vec2f& size);
	const Vec2f& GetSize() const;
	
	void Zoom(float factor);

	void SetViewport(const sf::FloatRect& viewport);

	const sf::View& GetView() const;

private:
	sf::View m_view;
	static inline std::vector<Camera*> m_cameras {};
	static inline Camera* m_activeCam = nullptr;
	bool m_active = false;
    Vec2f last_size;
};

}
