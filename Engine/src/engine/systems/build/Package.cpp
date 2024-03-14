#include "pch.h"

#include <nlohmann/json.hpp>

#include "package.h"

#include "engine/Engine.h"

namespace Toad
{
	namespace fs = std::filesystem;
	using json = nlohmann::json;

	Package::Package()
	{
	}

	Package::~Package()
	{
	}

	bool Package::CreatePackage(const fs::path& project_file_path, const fs::path& output_dir_path, const fs::path& build_system_file_path, const fs::path& engine_path)
	{
		const fs::path proj_dir = project_file_path.parent_path();
		const fs::path proj_bin_path = proj_dir / "bin" / "Release-windows-x86_64";
		const fs::path out_dir = output_dir_path / "packaged_game";
		const fs::path& proj_engine_dir = engine_path;

		if (proj_engine_dir.empty())
		{
			LOGERROR("[Package] engine path is empty");
			return false;
		}

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
			LOGERRORF("[Package] No Game folder found in {}", proj_dir);
			return false;
		}

		fs::path slnfile_path;
		for (const auto& entry : fs::directory_iterator(proj_dir))
		{
			if (entry.path().has_extension() && entry.path().extension() == ".sln")
			{
				slnfile_path = entry;
				break;
			}
		}

		if (slnfile_path.empty())
		{
			LOGERRORF("[Package] No .sln file found in {}", project_file_path.parent_path());
			return false;
		}

		if (project_file_path.extension() != FILE_EXT_TOADPROJECT)
		{
			LOGERRORF("[Package] Create package project file is invalid: {}", project_file_path);
			return false;
		}

		// delete log from before
		if (fs::exists(proj_dir / "buildlog.txt"))
		{
			fs::remove(proj_dir / "buildlog.txt");
		}

		LOGDEBUGF("[Package] Running build for {}", slnfile_path);
		if (!RunBuildSystemWithArgs(build_system_file_path, format_str("{} /build Release /out buildlog.txt", slnfile_path.string())))
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
			LOGERRORF("[Package] Experienced errors while building {}", slnfile_path);
			return false;
		}

		// check output binaries 
		bool has_dll = false;
		for (const auto& entry : fs::directory_iterator(proj_bin_path))
		{
			if (entry.path().extension() == ".dll")
			{
				has_dll = true;
				break;
			}
		}

		if (!has_dll)
		{
			LOGERRORF("[Package] No game dll found in {}", proj_bin_path);
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

#ifdef TOAD_DISTRO
		for (const auto& entry : fs::directory_iterator(proj_bin_path))
		{
			if (entry.path().extension() == ".dll")
			{
				fs::copy_file(entry.path(), out_dir / entry.path().filename());
			}
		}

		// others in bin (Engine) 
		for (const auto& entry : fs::directory_iterator(proj_engine_dir / "bin"))
		{
			if (entry.path().extension() == ".dll")
			{
				fs::copy_file(entry.path(), out_dir / entry.path().filename());
			}
		}

		fs::path bin = proj_engine_dir / "bin";

		// copy Runner No Editor
		std::ifstream proj_file_f(project_file_path);
		try {
			json data = json::parse(proj_file_f);
			std::string gamename = data["name"];
			fs::copy_file(bin / "ToadRunnerNoEditor.exe", out_dir / (gamename + ".exe"));
		}
		catch (json::parse_error& e)
		{
			LOGWARNF("Failed to read project file {}", project_file_path);
			fs::copy_file(bin / "ToadRunnerNoEditor.exe", out_dir / "ToadRunnerNoEditor.exe");
		}

		// sfml
		for (const auto& entry : fs::directory_iterator(proj_engine_dir))
		{
			if (entry.path().filename().extension() == ".dll")
			{
				if (entry.path().filename().string().find("GameCurrent") != std::string::npos) 
					continue;

				fs::copy_file(entry.path(), out_dir / entry.path().filename(), fs::copy_options::skip_existing);
			}
		}
#else 
		// has both runner and game 
		for (const auto& entry : fs::directory_iterator(proj_bin_path))
		{
			if (entry.path().extension() == ".exe" || entry.path().extension() == ".dll")
			{
				fs::copy(entry.path(), out_dir);
			}
		}

		for (const auto& entry : fs::directory_iterator(proj_engine_dir.parent_path() / "vendor"))
		{
			if (entry.path().filename().string().find("SFML") != std::string::npos)
			{
				for (const auto& entry2 : fs::directory_iterator(entry.path() / "bin"))
				{
					if (entry2.path().filename().string().find("-d-2") == std::string::npos)
					{
						fs::copy_file(entry2.path(), out_dir / entry2.path().filename());
					}
				}

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
