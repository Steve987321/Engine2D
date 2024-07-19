#pragma once

#include "engine/Types.h"
#include "nlohmann/json.hpp"

namespace Toad
{

// ids are relative paths
namespace ResourceManager
{
	template<typename T>
	class ENGINE_API ResourcesOfType
	{
	public:
		using TDATA = std::unordered_map<std::string, T>;
		
		explicit ResourcesOfType(std::string_view resource_name)
			: m_resourceName(resource_name)
		{}

	public:
		TDATA& GetData()
		{
			return m_data;
		}

		virtual T* Add(std::string_view id, const T& data)
		{
			if (m_data.contains(id.data()))
				LOGWARNF("[ResourceManager] Data of type '{}' with id '{}' already exists and is getting replaced", m_resourceName, id.data());

			m_data[id.data()] = T(data);
			return &m_data[id.data()];
		}

		virtual T* Get(std::string_view id)
		{
			auto it = m_data.find(id.data());
			if (it != m_data.end())
				return &(it->second);
			
			return nullptr;
		}

		// erases from map
		virtual bool Remove(std::string_view id)
		{
			auto it = m_data.find(id.data());
			if (it != m_data.end())
			{
				m_data.erase(it);
				return true;
			}

			return false;
		}

		virtual void Clear()
		{
			m_data.clear();
		}

	protected:
		TDATA m_data;
		std::string m_resourceName;
	};

	template class ResourcesOfType<sf::Texture>;
	template class ResourcesOfType<sf::Font>;
	template class ResourcesOfType<FSM>;

	class ENGINE_API AudioSourceResources : public ResourcesOfType<AudioSource>
	{
	public:
		AudioSourceResources(std::string_view name)
			: ResourcesOfType<AudioSource>(name)
		{}

		// will only clear the soundbuffer and will still hold the rest of the audiosource data
		bool Remove(std::string_view id) override;
	};

	ENGINE_API ResourcesOfType<sf::Texture>& GetTextures();
	ENGINE_API ResourcesOfType<sf::Font>& GetFonts();
	ENGINE_API AudioSourceResources& GetAudioSources();
	ENGINE_API ResourcesOfType<FSM>& GetFSMs();

	// clears all resources
	ENGINE_API void Clear();
}

}
