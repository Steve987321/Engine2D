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

	bool Package::CreatePackage(const fs::path& project_file, const fs::path& output_path, const fs::path& build_system_path)
	{
		const fs::path proj_dir = project_file.parent_path();
		fs::path proj_engine_dir;
		std::ifstream proj_file(project_file);

		if (proj_file.is_open())
		{
			try
			{
				json data = json::parse(proj_file);

				proj_engine_dir = fs::path(std::string(data["engine_path"]));
			}
			catch (json::parse_error& e)
			{
				proj_file.close();
				LOGERRORF("Parse error at {}, {}. While parsing {}", e.byte, e.what(), project_file);
				return false;
			}

			proj_file.close();
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

		if (project_file.extension() != ".TOADPROJECT")
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
					if (line.find("========== Build started at") != std::string::npos)
					{
						build_done = true;
						break;
					}
				}
				buildlog.clear();
				buildlog.seekg(0);
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}

		});

		while (!build_done)
		{
			// building status ..
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		if (check_build_done.joinable())
		{
			check_build_done.join();
		}

		fs::create_directory(out_dir);

		for (const auto& entry : fs::directory_iterator(proj_dir / "bin" / "Release-windows-x86_64"))
		{
			if (entry.path().extension() == ".exe" || entry.path().extension() == ".dll")
			{
				fs::copy(entry.path(), out_dir);
			}
		}

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
