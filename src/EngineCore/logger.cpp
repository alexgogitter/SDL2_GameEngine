#include "logger.hpp"
#include <SDL.h>
#include <chrono>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
namespace
{
std::mutex logMutex;
std::deque<LogEntry> history;
std::ofstream stream;
std::string logPath;
std::uint64_t sequence = 0;
void SDLCALL sdlLog(void *, int, SDL_LogPriority priority, const char *message)
{
    Logger::write(priority >= SDL_LOG_PRIORITY_ERROR ? LogLevel::Error : priority >= SDL_LOG_PRIORITY_WARN ? LogLevel::Warning : LogLevel::Info, "SDL", message ? message : "");
}
}
namespace Logger
{
bool initialize(const std::string &path)
{
    std::lock_guard<std::mutex> lock(logMutex);
    std::error_code error;
    auto p = std::filesystem::u8path(path);
    if (p.has_parent_path()) std::filesystem::create_directories(p.parent_path(), error);
    stream.close(); stream.clear(); stream.open(p, std::ios::app);
    logPath = path;
    SDL_LogSetOutputFunction(sdlLog, nullptr);
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);
    return stream.good();
}
void write(LogLevel level, const std::string &source, const std::string &message)
{
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::lock_guard<std::mutex> lock(logMutex);
    std::ostringstream stamp;
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    stamp << std::put_time(&local, "%Y-%m-%d %H:%M:%S");
    history.push_back({++sequence, level, stamp.str(), source, message});
    if (history.size() > 4000) history.pop_front();
    if (stream) {
        stream << stamp.str() << " [" << (level == LogLevel::Error ? "ERROR" : level == LogLevel::Warning ? "WARN" : "INFO") << "] [" << source << "] " << message << '\n';
        stream.flush();
    }
}
std::vector<LogEntry> entries() { std::lock_guard<std::mutex> lock(logMutex); return {history.begin(), history.end()}; }
void clearView() { std::lock_guard<std::mutex> lock(logMutex); history.clear(); }
std::string filePath() { std::lock_guard<std::mutex> lock(logMutex); return logPath; }
}
