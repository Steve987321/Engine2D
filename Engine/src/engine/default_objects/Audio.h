#pragma once

#include "Object.h"

namespace Toad
{

struct AudioSource;

class ENGINE_API Audio : public Object
{
public:
	explicit Audio(std::string_view obj_name);
	Audio(const Audio& other);
	~Audio() override;

	void Start() override;
	void Render(sf::RenderTarget& target) override;
	void Update() override;

	AudioSource* GetAudioSource() const;

	// Use GetAudioPosition 
	const Vec2f& GetPosition() const override;
	void SetPosition(const Vec2f& position) override;

	json Serialize() override;

	const sf::SoundBuffer& GetSoundBuffer() const;

public:
	// will already add resource in the resource manager
	void ShouldPlayFromSource(bool play_from_source);
	bool PlaysFromSource() const;

	void SetSource(AudioSource* source);

	sf::SoundSource::Status GetStatus() const;
	void Play();
	void Pause();
	void Stop();

	Vec3f GetAudioPosition() const;
	void SetAudioPosition(const Vec3f& position);

	void SetPitch(float pitch);
	float GetPitch() const;

	void SetVolume(float volume);
	float GetVolume() const;

	uint32_t GetChannels() const;

	sf::Time GetDuration() const;
	sf::Time GetTimeLine() const;
	void SetTimeLine(float second);

private:
	bool m_playFromSource = true;
	sf::Music m_music;
	sf::Sound m_sound;
	AudioSource* m_audioSource = nullptr;
	std::filesystem::path m_sourceFile;
};

}
