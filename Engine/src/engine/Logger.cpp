#include "pch.h"
#include "Logger.h"

#ifdef _WIN32
#include "ShlObj_core.h"
#endif

namespace Toad
{
	Logger::Logger()
	{
#ifdef _WIN32
		HWND console_window = GetConsoleWindow();
#ifndef TOAD_NO_CONSOLE_LOG
		if (!console_window)
		{
			AllocConsole();
			FILE* f;
			freopen_s(&f, "CONOUT$", "w", stdout);
			console_window = GetConsoleWindow();
		}

		m_stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
#endif 
		if (create_log_file)
		{
			// create log file in the documents folder
			std::string logFileName = "Toad.log";
			m_logFile.open(GetLogFolder() + "\\" + logFileName);

			// log the date in the beginning
			LogToFile(get_date_str("%Y %d %b \n"));
		}
#endif
	}

	Logger::~Logger()
	{
		DisposeLogger();
	}

	void Logger::DisposeLogger()
	{
		std::lock_guard lock(m_closeMutex);

		if (m_logFile.is_open())
			m_logFile.close();

#ifdef _WIN32
		if (m_stdoutHandle)
		{
			CloseHandle(m_stdoutHandle);
			m_stdoutHandle = nullptr;
		}
#endif
	}

	std::string Logger::GetLogFolder()
	{
		return get_exe_path().string();
	}

	void Logger::LogToFile(const std::string_view str)
	{
		m_logFile << str << std::endl;
	}
}
