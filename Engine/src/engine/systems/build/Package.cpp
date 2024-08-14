#include "pch.h"

#include <nlohmann/json.hpp>

#include "Package.h"

#include "engine/Engine.h"

namespace Toad
{
	namespace fs = std::filesystem;
	using json = nlohmann::ordered_json;

	Package::Package()
	{
	}

	Package::~Package()
	{
	}

	bool Package::CreatePackage(const CreatePackageParams& params)
	{
		const std::string build_config_arg = params.is_debug ? "Debug" : "Release";
		const std::string build_config_arg_lower = params.is_debug ? "debug" : "release";

		const fs::path proj_dir = params.project_file_path.parent_path();
		const fs::path proj_bin_path = proj_dir / "bin" / Toad::format_str("{}-{}-x86_64", build_config_arg, PLATFORM_AS_STRING);
		const fs::path out_dir = params.output_dir_path / "packaged_game";
		const fs::path& proj_engine_dir = params.engine_path;

		if (proj_engine_dir.empty())
		{
			LOGERROR("[Package] engine path is empty");
			return false;
		}

		// get assets path 
		fs::path proj_assets_path;
		for (const auto& entry : fs::directory_iterator(proj_dir))
		{
			if (entry.path().filename().string().find("_Game") != std::string::npos)
			{
				proj_assets_path = entry.path() / "src" / "assets";
				if (!fs::exists(proj_assets_path))
				{
					LOGERROR("[Package] No assets or src folder found in {}", proj_assets_path);
					return false;
				}

				break;
			}
		}

		if (proj_assets_path.empty())
		{
			LOGERRORF("[Package] No assets found in {}", proj_dir);
			return false;
		}

		fs::path project_file_path;
#ifdef _WIN32
		// look for sln 
		for (const auto& entry : fs::directory_iterator(proj_dir))
		{
			if (entry.path().has_extension() && entry.path().extension() == ".sln")
			{
				project_file_path = entry;
				break;
			}
		}

		if (project_file_path.empty())
		{
			LOGERRORF("[Package] No .sln file found in {}", params.project_file_path.parent_path());
			return false;
		}
#else
		// look for makefile
		for (const auto& entry : fs::directory_iterator(proj_dir))
		{
			if (entry.path().filename().string() == "Makefile")
			{
				project_file_path = entry;
				break;
			}
		}

		if (project_file_path.empty())
		{
			LOGERRORF("[Package] No Makefile file found in {}", params.project_file_path.parent_path());
			return false;
		}
#endif 

#ifdef _WIN32
		if (params.project_file_path.extension() != FILE_EXT_TOADPROJECT)
		{
			LOGERRORF("[Package] Create package project file is invalid: {}", params.project_file_path);
			return false;
		}

		// delete log from before
		if (fs::exists(proj_dir / "buildlog.txt"))
		{
			fs::remove(proj_dir / "buildlog.txt");
		}

		LOGDEBUGF("[Package] Running build for {}", project_file_path);
		if (!RunBuildSystemWithArgs(params.build_system_file_path, format_str("{} /build {} /out buildlog.txt", project_file_path.string(), build_config_arg)))
		{
			LOGERROR("Failed to run build system");
			return false;
		}

		while (!fs::exists(proj_dir / "buildlog.txt"))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		std::ifstream buildlog(proj_dir / "buildlog.txt");

		if (!buildlog)
		{
			LOGERRORF("[Package] buildlog.txt can't be opened in {}", proj_dir);
			return false;
		}

		std::atomic_bool build_done = false;
		std::string build_finish_status;

		std::set<std::string> build_progress_status;

		// reads buildlog.txt
		std::thread check_build_done([&]
		{
			while (!build_done)
			{
				std::string line;
				while (std::getline(buildlog, line))
				{
					if (!build_progress_status.contains(line))
					{
						LOGDEBUGF("[Package] Build Status: {}", line);
						build_progress_status.insert(line);
					}
					if (line.find("========== Build: ") != std::string::npos)
					{
						build_finish_status = line;
						build_done = true;
						break;
					}
				}
				buildlog.clear();
				buildlog.seekg(0);
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
		});

		while (!build_done)
		{
			// building status ..
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}

		if (check_build_done.joinable())
		{
			check_build_done.join();
		}

		// check build status 
		if (build_finish_status.find("0 failed") == std::string::npos)
		{
			LOGERRORF("[Package] Experienced errors while building {}", project_file_path);
			return false;
		}
#else 
		int build_res = system(Toad::format_str("cd {} && make config={}", project_file_path.parent_path(), build_config_arg_lower).c_str());

		if (build_res != 0)
		{
			LOGERRORF("[Package] Experienced errors while building {}", project_file_path);
			return false;
		}
#endif 
		// check output binaries 
		bool has_dll = false;
		for (const auto& entry : fs::directory_iterator(proj_bin_path))
		{
			if (entry.path().extension() == LIB_FILE_EXT)
			{
				has_dll = true;
				break;
			}
		}

		if (!has_dll)
		{
			LOGERRORF("[Package] No game dl file found in {}", proj_bin_path);
			return false;
		}

		if (!fs::exists(proj_assets_path))
		{
			LOGERRORF("[Package] No assets folder in {}", proj_dir);
			return false;
		}

		fs::create_directory(out_dir);

		// #TODO add compression
		// copy assets (no scripts)
		for (const auto& entry : fs::recursive_directory_iterator(proj_assets_path))
		{
			// skip source script files 
			if (entry.path().extension() == ".cpp" || entry.path().extension() == ".h")
				continue;

			fs::path destination = out_dir / fs::relative(entry.path(), proj_assets_path);

			if (fs::is_directory(entry))
			{
				fs::create_directories(destination);
			}
			else
			{
				try
				{
					fs::copy_file(entry.path(), destination, fs::copy_options::overwrite_existing);
				}
				catch (fs::filesystem_error& e)
				{
					LOGERRORF("[Package] Failed to copy {} to {} : {}", entry.path(), destination, e.what());
				}
			}
		}
		for (const auto& entry : fs::directory_iterator(proj_bin_path))
		{
			if (entry.path().extension() == LIB_FILE_EXT)
			{
				fs::copy_file(entry.path(), out_dir / entry.path().filename(), fs::copy_options::overwrite_existing);
			}
		}

#ifdef TOAD_DISTRO
		fs::path bin = params.is_debug ? proj_engine_dir / "bin" / "debug" : proj_engine_dir / "bin";
		fs::path runner = params.is_debug ? "ToadRunnerNoEditorDebug.exe" : "ToadRunnerNoEditor.exe";

		// others in bin (Engine) 
		for (const auto& entry : fs::directory_iterator(proj_engine_dir / "bin"))
		{
			if (entry.path().extension() == ".dll")
			{
				fs::copy_file(entry.path(), out_dir / entry.path().filename());
			}
		}

		// sfml 
		for (const auto& entry : fs::directory_iterator(params.is_debug ? bin : proj_engine_dir))
		{
			if (entry.path().string().find("sfml") != std::string::npos && entry.path().extension() == ".dll")
			{
				fs::copy_file(entry.path(), out_dir / entry.path().filename());
			}
		}

		// copy Runner No Editor
		std::ifstream proj_file_f(params.project_file_path);
		try {
			json data = json::parse(proj_file_f);
			std::string gamename = data["name"];
			fs::copy_file(bin / runner, out_dir / (gamename + ".exe"));
		}
		catch (json::parse_error& e)
		{
			LOGWARNF("Failed to read project file {}", params.project_file_path);
			fs::copy_file(bin / runner, out_dir / runner);
		}

		// sfml
		//for (const auto& entry : fs::directory_iterator(proj_engine_dir))
		//{
		//	if (entry.path().filename().extension() == ".dll")
		//	{
		//		if (entry.path().filename().string().find("GameCurrent") != std::string::npos)
		//			continue;

		//		fs::copy_file(entry.path(), out_dir / entry.path().filename(), fs::copy_options::skip_existing);
		//	}
		//}
#else 
		fs::path bin;
		if (params.is_debug)
			bin = proj_engine_dir / "bin" / format_str("DebugNoEditor-{}-x86_64", PLATFORM_AS_STRING);
		else
			bin = proj_engine_dir / "bin" / format_str("ReleaseNoEditor-{}-x86_64", PLATFORM_AS_STRING);

		std::string runner_name = format_str("ToadRunner{}", EXE_FILE_EXT);
		std::string engine_dl_name = format_str("{}Engine{}", LIB_FILE_PREFIX, LIB_FILE_EXT);
		fs::path runner = bin / runner_name;
		fs::path enginedll = bin / engine_dl_name;

		if (!fs::exists(bin))
		{
			LOGERRORF("[Package] Path doesn't exist: {}", bin);
			return false;
		}
		if (!fs::exists(runner))
		{
			LOGERRORF("[Package] Path doesn't exist: {}", runner);
			return false;
		}
		if (!fs::exists(enginedll))
		{
			LOGERRORF("[Package] Path doesn't exist: {}", enginedll);
			return false;
		}

		// #TODO rename runner to game project name
		fs::copy_file(runner, out_dir / runner_name, fs::copy_options::overwrite_existing);
		fs::copy_file(enginedll, out_dir / engine_dl_name, fs::copy_options::overwrite_existing);

		for (const auto& entry : fs::directory_iterator(proj_engine_dir / "vendor"))
		{
			if (entry.path().filename().string().find("SFML") != std::string::npos)
			{
#ifdef _WIN32
				for (const auto& entry2 : fs::directory_iterator(entry.path() / "bin"))
				{
					if (params.is_debug) 
					{
						if (entry2.path().filename().string().find("-d-2") != std::string::npos)
						{
							fs::copy_file(entry2.path(), out_dir / entry2.path().filename());
						}
					}
					else
					{
						if (entry2.path().filename().string().find("-d-2") == std::string::npos)
						{
							fs::copy_file(entry2.path(), out_dir / entry2.path().filename());
						}
					}
				}
#else
				for (const auto& entry2 : fs::directory_iterator(entry.path() / "lib"))
				{
					if (entry2.path().filename().string().find("sfml-") != std::string::npos && entry2.path().filename().extension() == ".dylib")
					{
						fs::copy_file(entry2.path(), out_dir / entry2.path().filename());
					}
				}
#endif

				break;
			}
		}

		for (const auto& entry : fs::recursive_directory_iterator(out_dir))
		{
			if (entry.path().extension() == ".h" || entry.path().extension() == ".cpp")
			{
				fs::remove(entry.path());
			}
		}
#endif 

		return true;
	}

	bool Package::RunBuildSystemWithArgs(const std::filesystem::path& build_program, std::string_view args)
	{
#ifdef _WIN32
		STARTUPINFOA si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		if (!CreateProcessA(NULL,
			const_cast<LPSTR>((build_program.string() + ' ' + args.data()).c_str()),
			NULL,
			NULL,
			FALSE,
			0,
			NULL,
			NULL,
			&si,
			&pi))
		{
			return false;
		}

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
#endif
		return true;
	}

}
