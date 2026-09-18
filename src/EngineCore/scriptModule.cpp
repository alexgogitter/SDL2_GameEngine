#include "scriptModule.hpp"
#include "scriptComponent.hpp"
#include "sceneSerialization.hpp"
#include "componentRegistry.hpp"
#include "builtInComponents.hpp"
#include "scene.hpp"
#include "logger.hpp"
#include <SDL.h>
#include <filesystem>
#include <chrono>
#include <stdexcept>
namespace fs = std::filesystem;
struct ScriptModule::Impl
{
    void *handle = nullptr;
    fs::path shadow;
    fs::file_time_type attempted{};
    std::uintmax_t attemptedSize = 0;
    std::chrono::steady_clock::time_point lastCheck{};
    ~Impl()
    {
        if (handle) SDL_UnloadObject(handle);
        std::error_code ignored;
        if (!shadow.empty()) fs::remove(shadow, ignored);
    }
};
ScriptModule::ScriptModule() : impl(std::make_unique<Impl>()) {}
ScriptModule::~ScriptModule() = default;
bool ScriptModule::reload(const std::string &path, Scene &scene, ComponentRegistry &registry, const ComponentCreateContext &context, bool force)
{
    const auto now = std::chrono::steady_clock::now();
    if (!force && now - impl->lastCheck < std::chrono::milliseconds(500)) return false;
    impl->lastCheck = now;
    std::error_code ec;
    const auto source = fs::u8path(path);
    const auto modified = fs::last_write_time(source, ec);
    if (ec) {
        if (force) Logger::write(LogLevel::Error, "Scripts", "Cannot read module: " + path);
        return false;
    }
    const auto size = fs::file_size(source, ec);
    if (ec || (!force && modified == impl->attempted && size == impl->attemptedSize)) return false;
    impl->attempted = modified; impl->attemptedSize = size;
    Impl candidate;
    try {
        auto directory = fs::temp_directory_path() / "RicochetScripts";
        fs::create_directories(directory);
        candidate.shadow = directory / ("UserScripts-" + std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()) + source.extension().string());
        fs::copy_file(source, candidate.shadow);
        if (fs::last_write_time(source) != modified || fs::file_size(source) != size)
            throw std::runtime_error("Module is still being written; retry after the build completes");
        candidate.handle = SDL_LoadObject(candidate.shadow.u8string().c_str());
        if (!candidate.handle) throw std::runtime_error(SDL_GetError());
        auto version = reinterpret_cast<unsigned (*)()>(SDL_LoadFunction(candidate.handle, "GetScriptApiVersion"));
        auto registerTypes = reinterpret_cast<bool (*)(ComponentRegistry *)>(SDL_LoadFunction(candidate.handle, "RegisterGameComponents"));
        if (!version || !registerTypes || version() != EngineScriptApiVersion)
            throw std::runtime_error("Script DLL has missing exports or an incompatible API version");
        ComponentRegistry replacement;
        if (!RegisterBuiltInComponents(replacement) || !registerTypes(&replacement))
            throw std::runtime_error("Script component registration failed");
        std::string error;
        if (!SceneSerialization::restore(scene, SceneSerialization::capture(scene), replacement, context, error))
            throw std::runtime_error(error);
        registry.swap(replacement);
        std::swap(impl->handle, candidate.handle);
        std::swap(impl->shadow, candidate.shadow);
        Logger::write(LogLevel::Info, "Scripts", "Loaded " + path + "; preserved hierarchy and serializable component properties");
        return true;
    } catch (const std::exception &e) {
        Logger::write(LogLevel::Error, "Scripts", std::string("Reload rejected; previous module remains active: ") + e.what());
        return false;
    }
}
