#include "pch.h"
#include "Audio.h"

#include "engine/Engine.h"

namespace Toad
{

Audio::Audio(std::string_view obj_name)
{
	name = obj_name;
}

Audio::Audio(const Audio& other)
	: Object(other)
{
	name = other.name;
	m_audioSource = other.m_audioSource;
	if (m_audioSource != nullptr)
	{
		m_music.openFromFile(m_audioSource->full_path.string());
	}
	m_sound = other.m_sound;
	m_playFromSource = other.m_playFromSource;
	m_sourceFile = other.m_sourceFile;
}

Audio::~Audio()
= default;

void Audio::Start()
{
	Object::Start();

	// update attached scripts
	for (auto script : m_attachedScripts | std::views::values)
	{
		script->OnStart(this);
	}
}

void Audio::Render(sf::RenderWindow& window)
{
	Object::Render(window);
}

void Audio::Render(sf::RenderTexture& texture)
{
	Object::Render(texture);
}

void Audio::Update()
{
	Object::Update();

	// update attached scripts
	for (auto script : m_attachedScripts | std::views::values)
	{
		script->OnUpdate(this);
	}
}

AudioSource* Audio::GetAudioSource() const
{
	return m_audioSource;
}

const Vec2f& Audio::GetPosition()
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
	json attached_scripts;

	for (const auto& it : GetAttachedScripts())
	{
		const auto& reflection_vars = it.second->GetReflection().Get();
		const auto& bs = reflection_vars.b;
		const auto& flts = reflection_vars.flt;
		const auto& i8s = reflection_vars.i8;
		const auto& i16s = reflection_vars.i16;
		const auto& i32s = reflection_vars.i32;
		const auto& strs = reflection_vars.str;

		json bs_data;
		for (const auto& [name, val] : bs)
		{
			bs_data[name] = *val;
		}
		json flts_data;
		for (const auto& [name, val] : flts)
		{
			flts_data[name] = *val;
		}
		json i8s_data;
		for (const auto& [name, val] : i8s)
		{
			i8s_data[name] = *val;
		}
		json i16s_data;
		for (const auto& [name, val] : i16s)
		{
			i16s_data[name] = *val;
		}
		json i32s_data;
		for (const auto& [name, val] : i32s)
		{
			i32s_data[name] = *val;
		}
		json strs_data;
		for (const auto& [name, val] : strs)
		{
			strs_data[name] = *val;
		}
		attached_scripts[it.first] =
		{
			bs_data,
			flts_data,
			i8s_data,
			i16s_data,
			i32s_data,
			strs_data
		};
	}

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

const sf::SoundBuffer* Audio::GetSoundBuffer() const
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
		Engine::Get().GetResourceManager().RemoveAudioSource(m_audioSource->relative_path.string());
		m_music.openFromFile(file_path.string());
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

		m_audioSource = Engine::Get().GetResourceManager().AddAudioSource(m_audioSource->relative_path.string(), *m_audioSource);
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
	m_sourceFile = source.relative_path;
#endif

	assert(!m_sourceFile.empty());

	if (!source->has_valid_buffer && !m_playFromSource)
	{
		source->sound_buffer.loadFromFile(m_sourceFile.string());
		source->has_valid_buffer = true;
	}

	m_music.openFromFile(m_sourceFile.string());
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
