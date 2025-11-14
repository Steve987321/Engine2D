#pragma once

#include "engine/Logger.h"

namespace Toad {

	class LogWindow
	{
    public: 
        LogWindow();
        ~LogWindow();

        void Show();
	private:
        size_t callback_id = 0;
        struct LogMsg
        {
            std::string msg;
            Logger::LOG_TYPE type;
        };

        std::map<LogMsg, size_t> logs_debug;
    };

}