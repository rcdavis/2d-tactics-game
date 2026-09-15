#include "Utils/Log.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

std::shared_ptr<spdlog::logger> Log::s_Logger;

void Log::Init(const std::string& tag) {
#if LOGGING_ENABLED
	const std::array<spdlog::sink_ptr, 2> logSinks = {
		std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
		std::make_shared<spdlog::sinks::basic_file_sink_mt>(tag + ".log", true),
	};

	logSinks[0]->set_pattern("%^[%T] %n: %v%$");
	logSinks[1]->set_pattern("[%T] [%l] %n: %v");

	s_Logger = std::make_shared<spdlog::logger>(tag, std::cbegin(logSinks), std::cend(logSinks));
	s_Logger->set_level(spdlog::level::trace);
	s_Logger->flush_on(spdlog::level::trace);
	spdlog::register_logger(s_Logger);
#endif
}
