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

	bool Package::CreatePackage(const fs::path& project_file, const fs::path& output_path, const fs::path& build_system_path, const fs::path& engine_path)
	{
		const fs::path proj_dir = project_file.parent_path();
		const fs::path& proj_engine_dir = engine_path;
		if (proj_engine_dir.empty())
		{
			LOGERROR("[Package] engine path is empty");
			return false;
		}

		fs::path slnfile_dir;

		for (const auto& entry : fs::directory_iterator(proj_dir))
		{
			if (entry.path().has_extension() && entry.path().extension() == ".sln")
			{
				slnfile_dir = entry;
				break;
			}
		}

		const fs::path out_dir = output_path / "packaged_game";

		if (project_file.extension() != FILE_EXT_TOADPROJECT)
		{
			LOGERRORF("Create package project file is invalid: {}", project_file);
			return false;
		}

		// delete log from before if exists
		if (fs::exists(proj_dir / "buildlog.txt"))
		{
			fs::remove(proj_dir / "buildlog.txt");
		}

		if (!RunBuildSystemWithArgs(build_system_path, format_str("{} /build Release /out buildlog.txt", slnfile_dir.string())))
		{
			LOGERROR("Failed to run build system");
			return false;
		}

		while (!fs::exists(proj_dir / "buildlog.txt"))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		std::ifstream buildlog(proj_dir / "buildlog.txt");
		std::atomic_bool build_done = false;

		if (!buildlog.is_open())
		{
			return false;
		}

		std::thread check_build_done([&]
		{
			while (!build_done)
			{
				std::string line;
				while (std::getline(buildlog, line))
				{
#ifdef TOAD_DISTRO
					if (line.find("1 up-to-date") != std::string::npos)
					{
						build_done = true;
						break;
					}
					if (line.find("========== Build: 0 succeeded, 1 failed") != std::string::npos)
					{
						build_done = true;
						return false;
					}

					if (line.find("========== Build: 1 succeeded") != std::string::npos
						&& line.find("0 failed") != std::string::npos)					
#else
					if (line.find("========== Build: 3 succeeded") != std::string::npos
						&& line.find("0 failed") != std::string::npos)
#endif 
					{
						build_done = true;
						break;
					}
				}
				static std::string line_last;
				if (line_last != line)
					LOGDEBUGF("[Package] {}", line);
				line_last = line;
				buildlog.clear();
				buildlog.seekg(0);
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}

		});

		while (!build_done)
		{
			// building status ..
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		if (check_build_done.joinable())
		{
			check_build_done.join();
		}

		fs::create_directory(out_dir);

		for (const auto& entry : fs::directory_iterator(proj_dir))
		{
			if (!entry.is_directory())
			{
				continue;
			}

			if (entry.path().filename().string().find("_Game") != std::string::npos)
			{
				fs::copy(entry.path() / "src" / "assets", out_dir, fs::copy_options::recursive);

				break;
			}
		}

#ifdef TOAD_DISTRO
		for (const auto& entry : fs::directory_iterator(proj_dir / "bin" / "Release-windows-x86_64"))
		{
			if (entry.path().extension() == ".dll")
			{
				fs::copy_file(entry.path(), out_dir / entry.path().filename());
			}
		}

		fs::path bin = proj_engine_dir / "bin";

		std::ifstream proj_file_f(project_file);
		try {
			json data = json::parse(proj_file_f);
			std::string gamename = data["name"];
			fs::copy_file(bin / "ToadRunnerNoEditor.exe", out_dir / (gamename + ".exe"));
		}
		catch (json::parse_error& e)
		{
			LOGWARNF("Failed to read project file {}", project_file);
			fs::copy_file(bin / "ToadRunnerNoEditor.exe", out_dir / "ToadRunnerNoEditor.exe");
		}

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
		for (const auto& entry : fs::directory_iterator(proj_dir / "bin" / "Release-windows-x86_64"))
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
