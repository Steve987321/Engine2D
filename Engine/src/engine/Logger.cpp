#include "pch.h"
#include "Logger.h"

#ifdef _WIN32
#include "ShlObj_core.h"
#endif

namespace Toad
{
	extern std::filesystem::path get_exe_path();

	Logger::Logger()
	{
#ifdef _WIN32
		HWND console_window = GetConsoleWindow();
		if (console_window)
		{
#ifdef TOAD_NO_CONSOLE_LOG
			ShowWindow(console_window, SW_HIDE);
#else 
			ShowWindow(console_window, SW_NORMAL);
#endif 
		}

#ifndef TOAD_NO_CONSOLE_LOG
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
