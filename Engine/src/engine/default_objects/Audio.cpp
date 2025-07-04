#include "pch.h"
#include "engine/Engine.h"
#include "Audio.h"

namespace Toad
{

static sf::SoundBuffer* GetDefaultSoundBuffer()
{
	AudioSource* audio_src = ResourceManager::GetAudioSources().Get("Default");
	assert(audio_src && "No default audio source in ResourceManager");
	return &audio_src->sound_buffer;
}

Audio::Audio(std::string_view obj_name)
	: m_sound(*GetDefaultSoundBuffer())
{
	name = obj_name;
}

Audio::Audio(const Audio& other)
	: Object(other), m_sound(*GetDefaultSoundBuffer())
{
	name = other.name;
	m_audioSource = other.m_audioSource;
	if (m_audioSource != nullptr)
	{
		bool load_success = m_music.openFromFile(m_audioSource->full_path.string());
		assert(load_success && "Failed to load audio file from path");
	}
	
	m_sound = other.m_sound;
	m_playFromSource = other.m_playFromSource;
	m_sourceFile = other.m_sourceFile;
}

Audio::~Audio() = default;

void Audio::Start()
{
	Object::Start();
}

void Audio::Render(sf::RenderTarget& target)
{
	Object::Render(target);
}

void Audio::Update()
{
	Object::Update();
}

AudioSource* Audio::GetAudioSource() const
{
	return m_audioSource;
}

const Vec2f& Audio::GetPosition() const 
{
	return Object::GetPosition();
}

void Audio::SetPosition(const Vec2f& position)
{
	Object::SetPosition(position);
}

json Audio::Serialize()
{
	json a_data;
	json audio_properties;
	json attached_scripts = SerializeScripts();

	audio_properties["parent"] = GetParent();

	audio_properties["posx"] = GetPosition().x;
	audio_properties["posy"] = GetPosition().y;

	audio_properties["play_from_source"] = m_playFromSource;

	audio_properties["volume"] = m_sound.getVolume();
	audio_properties["pitch"] = m_sound.getPitch();
	audio_properties["audio_posx"] = m_sound.getPosition().x;
	audio_properties["audio_posy"] = m_sound.getPosition().y;
	audio_properties["audio_posz"] = m_sound.getPosition().z;
	if (m_audioSource != nullptr)
	{
		json audio_source;
		audio_source["full_path"] = m_audioSource->full_path.string();
		audio_source["rel_path"] = m_audioSource->relative_path.string();
		audio_source["has_valid_buf"] = m_audioSource->has_valid_buffer;
		audio_properties["audio_source"] = audio_source;
	}
	a_data["properties"] = audio_properties;
	a_data["scripts"] = attached_scripts;

	return a_data;
}

const sf::SoundBuffer& Audio::GetSoundBuffer() const
{
	return m_sound.getBuffer();
}

void Audio::ShouldPlayFromSource(bool play_from_source)
{
	if (m_playFromSource == play_from_source)
	{
		return;
	}

	m_playFromSource = play_from_source;

	if (m_audioSource == nullptr)
	{
		return;
	}

#ifdef TOAD_EDITOR
	const std::filesystem::path& file_path = m_audioSource->full_path;
#else
	const std::filesystem::path& file_path = m_audioSource->relative_path;
#endif

	if (m_playFromSource)
	{
		ResourceManager::GetAudioSources().Remove(m_audioSource->relative_path.string());
		bool load_success = m_music.openFromFile(file_path.string());
		assert(load_success && "Failed to load audio file from path");
	}
	else
	{
		sf::SoundBuffer new_sound_buffer;
		if (!new_sound_buffer.loadFromFile(file_path.string()))
		{
			LOGERRORF("[Audio:{}] Failed to load {}", name, m_sourceFile);
			return;
		}

		m_audioSource->sound_buffer = sf::SoundBuffer(new_sound_buffer);

		m_audioSource = ResourceManager::GetAudioSources().Add(m_audioSource->relative_path.string(), *m_audioSource);
		m_sound.setBuffer(m_audioSource->sound_buffer);
		m_audioSource->has_valid_buffer = true;
	}
}

bool Audio::PlaysFromSource() const
{
	return m_playFromSource;
}

void Audio::SetSource(AudioSource* source)
{
#ifdef TOAD_EDITOR
	m_sourceFile = source->full_path;
#else
	m_sourceFile = source->relative_path;
#endif
        
	assert(!m_sourceFile.empty());
    
	if (!source->has_valid_buffer && !m_playFromSource)
	{
        bool opened_file = source->sound_buffer.loadFromFile(m_sourceFile.string());
		source->has_valid_buffer = opened_file;
	}

    bool opened_file = m_music.openFromFile(m_sourceFile.string());

    if (!opened_file)
    {
        LOGERRORF("[Audio] Failed to open '{}'. Setting default audiosource", m_sourceFile.string());
        
        m_sound.setBuffer(*GetDefaultSoundBuffer());
        m_playFromSource = false;
    }
    else
        m_sound.setBuffer(source->sound_buffer);
    
	m_audioSource = source;
}

sf::SoundSource::Status Audio::GetStatus() const
{
	if (m_playFromSource)
	{
		return m_music.getStatus();
	}
	else
	{
		return m_sound.getStatus();
	}
}

void Audio::Play()
{
	if (m_playFromSource)
	{
		m_music.play();
	}
	else
	{
		m_sound.play();
	}
}

void Audio::Pause()
{
	if (m_playFromSource)
	{
		m_music.pause();
	}
	else
	{
		m_sound.pause();
	}
}

void Audio::Stop()
{
	if (m_playFromSource)
	{
		m_music.stop();
	}
	else
	{
		m_sound.stop();
	}
}

Vec3f Audio::GetAudioPosition() const
{
	if (m_playFromSource)
	{
		return m_music.getPosition();
	}

	return m_sound.getPosition();
}

void Audio::SetAudioPosition(const Vec3f& position)
{
	m_music.setPosition(position);
	m_sound.setPosition(position);
}

void Audio::SetPitch(float pitch)
{
	m_sound.setPitch(pitch);
	m_music.setPitch(pitch);
}

float Audio::GetPitch() const
{
	return m_sound.getPitch();
}

void Audio::SetVolume(float volume)
{
	m_sound.setVolume(volume);
	m_music.setVolume(volume);
}

float Audio::GetVolume() const
{
	return m_sound.getVolume();
}

uint32_t Audio::GetChannels() const
{
	if (m_playFromSource)
	{
		return m_music.getChannelCount();
	}

	if (m_audioSource != nullptr)
	{
		return m_audioSource->sound_buffer.getChannelCount();
	}

	return 0;
}

sf::Time Audio::GetDuration() const
{
	if (m_playFromSource)
	{
		return m_music.getDuration();
	}

	if (m_audioSource != nullptr)
	{
		return m_audioSource->sound_buffer.getDuration();
	}

	return {};
}

sf::Time Audio::GetTimeLine() const
{
	if (m_playFromSource)
	{
		return m_music.getPlayingOffset();
	}

	return m_sound.getPlayingOffset();
}

void Audio::SetTimeLine(float second)
{
	sf::Time time = sf::seconds(second);
	if (m_playFromSource)
	{
		m_music.setPlayingOffset(time);
	}
	else
	{
		m_sound.setPlayingOffset(time);
	}
}

}
