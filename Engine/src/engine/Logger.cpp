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
		AllocConsole();

		m_stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		if (create_log_file)
		{
			// create log file in the documents folder
			std::string logFileName = "Toad.log";
			m_logFile.open(GetDocumentsFolder() + "\\" + logFileName, std::fstream::out);

			// log the date in the beginning
			LogToFile(GetDateStr("%Y %d %b \n"));
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

		if (m_isConsoleClosed) 
			return;
#ifdef _WIN32
		CloseHandle(m_stdoutHandle);
		m_stdoutHandle = nullptr;

		FreeConsole();
#endif

		m_isConsoleClosed = true;
	}

	std::string Logger::GetDateStr(const std::string_view format)
	{
		std::ostringstream ss;
		std::string time;

		auto t = std::time(nullptr);
		tm newtime{};

#ifdef _WIN32
		localtime_s(&newtime, &t);
#else
		localtime_r(&t, &newtime);
#endif

		ss << std::put_time(&newtime, format.data());
		return ss.str();
	}

	std::string Logger::GetDocumentsFolder()
	{
#ifdef _WIN32
		CHAR documents[MAX_PATH];
		HRESULT res = SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, documents);
		if (res == S_OK)
		{
			return documents;
		}
#endif

		return "";
	}

	void Logger::LogToFile(const std::string_view str)
	{
		m_logFile << str << std::endl;
	}
}
