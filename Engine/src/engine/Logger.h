#pragma once

#include "EngineCore.h"
#include "FormatStr.h"

#include <fstream>
#include <format>

#include "utils/Helpers.h"

#ifdef ERROR
#undef ERROR
#endif

static bool create_log_file = false;

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

	static Logger& Get();
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

	using FLOG_CALLBACK = std::function<void(LOG_TYPE, std::string_view msg)>;

public:
	/// Closes console and log file 
	void DisposeLogger();

	void AddCallback(const FLOG_CALLBACK& callback);

	// Clears stored logs that should only be logged once
	void ClearLogOnceMessages();

public:
	template <typename ... Args>
	void LogDebug(std::string_view frmt, bool log_once, Args... args)
	{
		Log(frmt, LOG_TYPE::DEBUG, log_once, args...);
	}

	template <typename ... Args>
	void LogWarning(std::string_view frmt, bool log_once, Args... args)
	{
		Log(frmt, LOG_TYPE::WARNING, log_once, args...);
	}

	template <typename ... Args>
	void LogError(std::string_view frmt, bool log_once, Args... args)
	{
		Log(frmt, LOG_TYPE::ERROR, log_once, args...);
	}

	template <typename ... Args>
	void LogException(std::string_view frmt, bool log_once, Args... args)
	{
		Log(frmt, LOG_TYPE::EXCEPTION, log_once, args...);
	}

private:
	/// Returns the directory location to the Documents folder 
	static std::string GetLogFolder();

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
	void Log(const std::string_view frmt, LOG_TYPE log_type, bool log_once, Args&& ... args)
	{
		std::lock_guard lock(m_mutex);

		std::string formatted_str = format_str(frmt, args...);

		if (log_once)
		{
			if (m_loggedOnceMessages.contains(formatted_str))
				return;

			m_loggedOnceMessages.emplace(formatted_str);
		}

		if (create_log_file)
			LogToFile(get_date_str("[%T]") + ' ' + formatted_str);

		for (const FLOG_CALLBACK& f : m_callbacks)
		{
			f(log_type, frmt);
		}

#ifndef TOAD_NO_CONSOLE_LOG
		Print(formatted_str, log_type);
#endif 
	}

private:
#ifdef _WIN32
	HANDLE m_stdoutHandle = nullptr;
#endif

	std::vector<FLOG_CALLBACK> m_callbacks;
	std::mutex m_mutex{};
	std::mutex m_closeMutex{};

	std::ofstream m_logFile{};

	// set of (formatted) logs that should only be logged once
	std::set<std::string> m_loggedOnceMessages{};
};

}

#define LOGDEBUGF(msg, ...) Toad::Logger::Get().LogDebug(msg, false, __VA_ARGS__)
#define LOGERRORF(msg, ...) Toad::Logger::Get().LogError(msg, false, __VA_ARGS__) 
#define LOGWARNF(msg, ...) Toad::Logger::Get().LogWarning(msg, false, __VA_ARGS__) 
#define LOGDEBUG(msg, ...) Toad::Logger::Get().LogDebug(msg, false, nullptr)
#define LOGERROR(msg, ...) Toad::Logger::Get().LogError(msg, false, nullptr) 
#define LOGWARN(msg, ...) Toad::Logger::Get().LogWarning(msg, false, nullptr) 
#define LOGONCEDEBUGF(msg, ...) Toad::Logger::Get().LogDebug(msg, true, __VA_ARGS__)
#define LOGONCEERRORF(msg, ...) Toad::Logger::Get().LogError(msg, true, __VA_ARGS__) 
#define LOGONCEWARNF(msg, ...) Toad::Logger::Get().LogWarning(msg, true, __VA_ARGS__) 
#define LOGONCEDEBUG(msg, ...) Toad::Logger::Get().LogDebug(msg, true, nullptr)
#define LOGONCEERROR(msg, ...) Toad::Logger::Get().LogError(msg, true, nullptr) 
#define LOGONCEWARN(msg, ...) Toad::Logger::Get().LogWarning(msg, true, nullptr) 
