#include "pch.h"
// #include "LogWindow.h"
// #include "engine/Logger.h"
// #include "engine/utils/Helpers.h"

// namespace Toad
// {
    
// DEFINE_ENUM_FLAG_OPERATORS(Logger::LOG_TYPE);

// static Logger::LOG_TYPE filters;

// static void log_callback(Logger::LOG_TYPE log_type, std::string_view msg)
// {
//     logs.emplace({msg, log_type});
// }

// LogWindow::LogWindow()
// {
//     callback_id = Logger::Get().AddCallback(log_callback);
// }
// LogWindow::~LogWindow()
// {
//     Logger::Get().RemoveCallback(callback_id);
// }

// void LogWindow::Show()
// {
//     static bool filter_errors = false;
//     static bool filter_warnings = false;
//     static bool filter_debug = false;

//     ImGui::Begin("Logs");
//     if (ImGui::Checkbox("err", &filter_errors))
//     {
//         if (filter_errors)
//         {
//             logs.
//         }
//     }
//     if (ImGui::Checkbox("warn", &filter_warnings))
//     {

//     }
//     if (ImGui::Checkbox("debug", &filter_debug))
//     {

//     }

//     for (auto& [log, count] : logs)
//     {
//         log.type
//     }

//     ImGui::End();
// }

// }
