#pragma once
#include "engineApi.hpp"
#include <cstdint>
#include <string>
#include <vector>
enum class LogLevel { Info, Warning, Error };
struct LogEntry
{
    std::uint64_t sequence = 0;
    LogLevel level = LogLevel::Info;
    std::string time, source, message;
};
/// Thread-safe session log, flushed to disk on every entry; retains the latest 4000 entries for UI.
namespace Logger
{
ENGINE_API bool initialize(const std::string &path);
ENGINE_API void write(LogLevel level, const std::string &source, const std::string &message);
ENGINE_API std::vector<LogEntry> entries();
ENGINE_API void clearView();
ENGINE_API std::string filePath();
}
