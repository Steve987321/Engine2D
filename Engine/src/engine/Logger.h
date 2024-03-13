#pragma once

#include "EngineCore.h"
#include "FormatStr.h"

#include <fstream>
#include <format>

#include "Helpers.h"

#ifdef ERROR
#undef ERROR
#endif

static bool create_log_file = true;

namespace Toad
{

///
/// Handles logs and console window
///
class ENGINE_API Logger final
{
public:
	Logger();
	~Logger();

public:
	enum class CONSOLE_COLOR : unsigned short
	{
		GREY = 8,
		WHITE = 15,
		RED = 12,
		GREEN = 10,
		BLUE = 9,
		YELLOW = 14,
		MAGENTA = 13,
	};

	enum class LOG_TYPE : unsigned short
	{
		DEBUG = static_cast<unsigned short>(CONSOLE_COLOR::BLUE),
		ERROR = static_cast<unsigned short>(CONSOLE_COLOR::RED),
		WARNING = static_cast<unsigned short>(CONSOLE_COLOR::YELLOW),
		EXCEPTION = static_cast<unsigned short>(CONSOLE_COLOR::MAGENTA)
	};

	std::unordered_map<LOG_TYPE, const char*> logTypeAsStr
	{
		{LOG_TYPE::DEBUG, "DEBUG"},
		{LOG_TYPE::ERROR, "ERROR"},
		{LOG_TYPE::EXCEPTION, "EXCEPTION"},
		{LOG_TYPE::WARNING, "WARNING"},
	};

public:
	/// Closes console and log file 
	void DisposeLogger();

public:
	template <typename ... Args>
	void LogDebug(const char* frmt, Args... args)
	{
		Log(frmt, LOG_TYPE::DEBUG, args...);
	}

	template <typename ... Args>
	void LogWarning(const char* frmt, Args... args)
	{
		Log(frmt, LOG_TYPE::WARNING, args...);
	}

	template <typename ... Args>
	void LogError(const char* frmt, Args... args)
	{
		Log(frmt, LOG_TYPE::ERROR, args...);
	}

	template <typename ... Args>
	void LogException(const char* frmt, Args... args)
	{
		Log(frmt, LOG_TYPE::EXCEPTION, args...);
	}

private:
	/// Returns the directory location to the Documents folder 
	static std::string GetDocumentsFolder();

	/// Writes to created log file
	void LogToFile(const std::string_view str);

private:
	/// Outputs string to console 
	void Print(const std::string_view str, LOG_TYPE log_type)
	{
#ifdef _WIN32
		std::cout << '[';

		SetConsoleTextAttribute(m_stdoutHandle, static_cast<unsigned short>(log_type));
		std::cout << logTypeAsStr[log_type];

		SetConsoleTextAttribute(m_stdoutHandle, static_cast<unsigned short>(CONSOLE_COLOR::WHITE));
		std::cout << "] [";

		SetConsoleTextAttribute(m_stdoutHandle, static_cast<unsigned short>(CONSOLE_COLOR::GREY));
		std::cout << get_date_str("%H:%M:%S");

		SetConsoleTextAttribute(m_stdoutHandle, static_cast<unsigned short>(CONSOLE_COLOR::WHITE));
		std::cout << "] ";

		SetConsoleTextAttribute(m_stdoutHandle, static_cast<unsigned short>(CONSOLE_COLOR::GREY));
		std::cout << str << std::endl;

		SetConsoleTextAttribute(m_stdoutHandle, static_cast<unsigned short>(CONSOLE_COLOR::WHITE));
#else
		std::cout << '[' << logTypeAsStr[log_type] << "] [" << get_date_str("%H:%M:%S") << "] " << str << std::endl;
#endif
	}

	/// Logs formatted string to console and log file
	///
	///	@param frmt Formatted string that gets formatted with the arguments using '{}'
	///	@param log_type Type of log that affects console colors and beginning message of output
	/// @param args Arguments that fit with the formatted string
	template<typename ... Args>
	void Log(const std::string_view frmt, LOG_TYPE log_type, Args&& ... args)
	{
		std::lock_guard lock(m_mutex);

		auto formattedStr = format_str(frmt, args...);

		if (create_log_file)
			LogToFile(get_date_str("[%T]") + ' ' + formattedStr);

#ifdef TOAD_NO_CONSOLE_LOG
		Print(formattedStr, log_type);
#endif 
	}

private:
#ifdef _WIN32
	HANDLE m_stdoutHandle{};
#endif

	std::mutex m_mutex{};
	std::mutex m_closeMutex{};

	std::atomic_bool m_isConsoleClosed = false;

	std::ofstream m_logFile{};
};

}

#define LOGDEBUGF(msg, ...) Toad::Engine::GetLogger().LogDebug(msg, __VA_ARGS__)
#define LOGERRORF(msg, ...) Toad::Engine::GetLogger().LogError(msg, __VA_ARGS__) 
#define LOGWARNF(msg, ...) Toad::Engine::GetLogger().LogWarning(msg, __VA_ARGS__) 
#define LOGDEBUG(msg, ...) Toad::Engine::GetLogger().LogDebug(msg, nullptr)
#define LOGERROR(msg, ...) Toad::Engine::GetLogger().LogError(msg, nullptr) 
#define LOGWARN(msg, ...) Toad::Engine::GetLogger().LogWarning(msg, nullptr) 
