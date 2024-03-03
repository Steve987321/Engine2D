#pragma once

#ifdef ERROR 
#undef ERROR 
#endif 

#ifndef DEFINE_ENUM_FLAG_OPERATORS
#define DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE) \
extern "C++" { \
inline constexpr ENUMTYPE operator | (ENUMTYPE a, ENUMTYPE b) noexcept { return ENUMTYPE(((int)a) | ((int)b)); } \
inline ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE b) noexcept { return (ENUMTYPE &)(((int &)a) |= ((int)b)); } \
inline constexpr ENUMTYPE operator & (ENUMTYPE a, ENUMTYPE b) noexcept { return ENUMTYPE(((int)a) & ((int)b)); } \
inline ENUMTYPE &operator &= (ENUMTYPE &a, ENUMTYPE b) noexcept { return (ENUMTYPE &)(((int &)a) &= ((int)b)); } \
inline constexpr ENUMTYPE operator ~ (ENUMTYPE a) noexcept { return ENUMTYPE(~((int)a)); } \
inline constexpr ENUMTYPE operator ^ (ENUMTYPE a, ENUMTYPE b) noexcept { return ENUMTYPE(((int)a) ^ ((int)b)); } \
inline ENUMTYPE &operator ^= (ENUMTYPE &a, ENUMTYPE b) noexcept { return (ENUMTYPE &)(((int &)a) ^= ((int)b)); } \
}
// from winnt.h
#endif

#include "Misc.h"
#include "nlohmann/json.hpp"

namespace misc
{
	struct Editor;
}

namespace project {

	enum class CREATE_PROJECT_RES
	{
		OK,
		PATH_NOT_EXIST,
		INVALID_PROJECT_SETTINGS,
		ERROR
	};

	inline std::ostream& operator<<(std::ostream& os, CREATE_PROJECT_RES& r)
	{
		switch(r){
		case CREATE_PROJECT_RES::OK:
			os << "OK";
			break;
		case CREATE_PROJECT_RES::PATH_NOT_EXIST: 
			os << "PATH_NOT_EXIST";
			break;
		case CREATE_PROJECT_RES::INVALID_PROJECT_SETTINGS:
			os << "INVALID_PROJECT_SETTINGS";
			break;
		case CREATE_PROJECT_RES::ERROR:
			os << "ERROR";
			break;
		}

		return os;
	}

	struct CREATE_PROJECT_RES_INFO
	{
		CREATE_PROJECT_RES res; 
		std::string description;
	};

	enum class PROJECT_FLAGS{
		NO_VENDOR_COPY = 1 << 0,		// Copies only specified files from vendor to project
		NO_DEFAULT_SCENE = 1 << 1,		// Doesn't copy default game scene to project, blank
        NO_DEFAULT_SCRIPTS = 1 << 1,	// Doesn't copy default game scripts to project
	};
	DEFINE_ENUM_FLAG_OPERATORS(PROJECT_FLAGS)

    enum class PROJECT_TYPE{
        VS_2022,
        VS_2019,
        VS_2015,
        Makefile,
        Codelite,
        Xcode,
    };

	enum class LOAD_PROJECT_RES
	{
		OK,
		DOESNT_EXIST,
		INVALID_PROJECT_FILE,
	};
	
	struct LOAD_PROJECT_RES_INFO 
	{
		LOAD_PROJECT_RES res;
		std::string description;
	};

	struct ProjectSettings
	{
		std::string name;
		std::string project_path;
		std::string engine_path;
		PROJECT_FLAGS project_flags;
        PROJECT_TYPE project_gen_type;

		nlohmann::json to_json() const
		{
			nlohmann::json data;
			data["name"] = name;
			data["project_path"] = project_path;
			return data;
		}
	};

	// active project 
	inline ProjectSettings current_project{};

#ifdef _WIN32
	bool OpenSln(const std::filesystem::path& settings, const misc::Editor& editor = misc::current_editor);
#endif

    std::string ProjectTypeAsStr(PROJECT_TYPE r);

	CREATE_PROJECT_RES_INFO Create(const ProjectSettings& settings);

	LOAD_PROJECT_RES_INFO Load(const std::string_view path);

	// rerun premake 
	bool Update(const ProjectSettings& settings, const std::filesystem::path& path);
}
