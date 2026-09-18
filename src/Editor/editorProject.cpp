#include "editorProject.hpp"
#include "scene.hpp"
#include "sceneSerialization.hpp"
#include "componentRegistry.hpp"
#include "editorSelection.hpp"
#include "editorPlayState.hpp"
#include "editorPreferences.hpp"
#include "layerRegistry.hpp"
#include "cameraSystem.hpp"
#include "cameraComponent.hpp"
#include "scriptModule.hpp"
#include "logger.hpp"
#include "object.hpp"
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <cstring>
#include <cctype>
#include <set>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
namespace fs = std::filesystem;
using Json = nlohmann::json;
struct EditorProject::Impl
{
    Scene &scene;
    ComponentRegistry &registry;
    const ComponentCreateContext &context;
    EditorSelection &selection;
    EditorPlayState &play;
    LayerRegistry &layers;
    CameraSystem &cameras;
    ScriptModule &scripts;
    fs::path directory = "res";
    std::string scenePath = "res/scenes/main.scene";
    std::string selectedAsset;
    char path[1024] = "res/scenes/main.scene";
    char filter[256] = "";
    char scriptName[128] = "NewScript";
    int action = 0;
    bool openDialog = false;
    bool openNewScriptDialog = false;
    bool sourceCheckRequested = false;
    std::string sourceFingerprint;
    ImGuiTextFilter logFilter;
    bool info = true, warnings = true, errors = true, follow = true;
    std::string buildLine;
#ifdef _WIN32
    HANDLE process = nullptr, pipe = nullptr;
#endif
    Impl(Scene &s, ComponentRegistry &r, const ComponentCreateContext &c, EditorSelection &sel,
         EditorPlayState &p, LayerRegistry &l, CameraSystem &cam, ScriptModule &mod)
        : scene(s), registry(r), context(c), selection(sel), play(p), layers(l), cameras(cam), scripts(mod) {}
    ~Impl()
    {
#ifdef _WIN32
        if (pipe) CloseHandle(pipe);
        if (process) CloseHandle(process);
#endif
    }
    void report(bool success, const std::string &message, const std::string &error)
    {
        Logger::write(success ? LogLevel::Info : LogLevel::Error, "Editor", success ? message : error);
    }
    void dialog(int kind, const std::string &defaultPath)
    {
        action = kind; openDialog = true;
        std::snprintf(path, sizeof(path), "%s", defaultPath.c_str());
    }
    void emitBuildLine()
    {
        if (buildLine.empty()) return;
        auto lower = buildLine;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        auto level = lower.find("error") != std::string::npos || lower.find("failed") != std::string::npos ? LogLevel::Error :
                     lower.find("warning") != std::string::npos ? LogLevel::Warning : LogLevel::Info;
        Logger::write(level, "Build", buildLine); buildLine.clear();
    }
    void build()
    {
#ifdef _WIN32
        if (process) return;
        SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
        HANDLE output = nullptr;
        if (!CreatePipe(&pipe, &output, &security, 0)) { report(false, "", "Cannot create build output pipe"); return; }
        SetHandleInformation(pipe, HANDLE_FLAG_INHERIT, 0);
        STARTUPINFOW startup{}; startup.cb = sizeof(startup);
        startup.dwFlags = STARTF_USESTDHANDLES;
        startup.hStdOutput = output; startup.hStdError = output;
        startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        PROCESS_INFORMATION child{};
        const auto command = fs::u8path(std::string("\"") + ENGINE_CMAKE_COMMAND + "\" \"-DSOURCE_DIR=" + ENGINE_SOURCE_DIRECTORY + "\" \"-DBUILD_DIR=" + ENGINE_BUILD_DIRECTORY + "\" -DCONFIG=" + ENGINE_BUILD_CONFIG + " -P \"" + ENGINE_SOURCE_DIRECTORY + "/cmake/BuildScripts.cmake\"").wstring();
        std::vector<wchar_t> mutableCommand(command.begin(), command.end()); mutableCommand.push_back(0);
        const bool started = CreateProcessW(nullptr, mutableCommand.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &startup, &child) != 0;
        CloseHandle(output);
        if (!started) { CloseHandle(pipe); pipe = nullptr; report(false, "", "Could not start CMake script build"); return; }
        process = child.hProcess; CloseHandle(child.hThread);
        Logger::write(LogLevel::Info, "Build", "Building C++ user scripts...");
#else
        report(false, "", "Build UserScripts using CMake, then select Reload Scripts");
#endif
    }

    std::string scriptFingerprint() const
    {
        std::error_code error;
        std::vector<std::string> entries;

        for (fs::directory_iterator iterator("scripts", error), end; !error && iterator != end; iterator.increment(error)) {
            if (!iterator->is_regular_file(error) || iterator->path().extension() != ".cpp") {
                continue;
            }

            entries.push_back(iterator->path().generic_u8string() + ":" +
                              std::to_string(fs::file_size(iterator->path(), error)) + ":" +
                              std::to_string(fs::last_write_time(iterator->path(), error).time_since_epoch().count()));
        }

        std::sort(entries.begin(), entries.end());
        std::string result;
        for (const std::string &entry : entries) {
            result += entry + "\n";
        }

        return result;
    }

    bool validScriptName() const
    {
        if (scriptName[0] == '\0' || !(std::isalpha(static_cast<unsigned char>(scriptName[0])) || scriptName[0] == '_')) {
            return false;
        }

        for (const char *character = scriptName + 1; *character != '\0'; ++character) {
            if (!(std::isalnum(static_cast<unsigned char>(*character)) || *character == '_')) {
                return false;
            }
        }

        return true;
    }

    bool createScript(std::string &error)
    {
        if (!validScriptName()) {
            error = "Use a C++ identifier: letters, digits, and underscores; do not start with a digit.";
            return false;
        }

        const fs::path filePath = fs::path("scripts") / (std::string(scriptName) + ".cpp");

        if (fs::exists(filePath)) {
            error = filePath.generic_u8string() + " already exists.";
            return false;
        }

        const std::string typeName = scriptName;
        const std::string source =
            "#include \"scriptComponent.hpp\"\n\n"
            "#include \"object.hpp\"\n\n"
            "class " + typeName + " final : public ScriptComponent\n"
            "{\n"
            "  public:\n"
            "    explicit " + typeName + "(Object *owner) : ScriptComponent(\"" + typeName + "\", owner) {}\n\n"
            "  protected:\n"
            "    void Setup() override {}\n"
            "    void PreUpdate(std::uint64_t milliseconds) override {}\n"
            "    void Update(std::uint64_t milliseconds) override {}\n"
            "    void Destroy() override {}\n"
            "};\n\n"
            "REGISTER_USER_SCRIPT(" + typeName + ")\n";

        if (!SceneSerialization::writeFile(filePath.generic_u8string(), source, error)) {
            return false;
        }

        sourceFingerprint = scriptFingerprint();
        sourceCheckRequested = false;
        Logger::write(LogLevel::Info, "Scripts", "Created " + filePath.generic_u8string());
        build();
        return true;
    }
    bool building() const
    {
#ifdef _WIN32
        return process != nullptr;
#else
        return false;
#endif
    }
    void pollBuild()
    {
#ifdef _WIN32
        if (!process) return;
        DWORD available = 0;
        while (PeekNamedPipe(pipe, nullptr, 0, nullptr, &available, nullptr) && available > 0) {
            char buffer[4096]; DWORD count = 0;
            if (!ReadFile(pipe, buffer, (std::min)(available, DWORD(sizeof(buffer))), &count, nullptr)) break;
            for (DWORD i = 0; i < count; ++i) {
                if (buffer[i] == '\n') emitBuildLine();
                else if (buffer[i] != '\r') buildLine += buffer[i];
            }
        }
        if (WaitForSingleObject(process, 0) != WAIT_OBJECT_0) return;
        emitBuildLine();
        DWORD result = 1; GetExitCodeProcess(process, &result);
        CloseHandle(process); CloseHandle(pipe); process = nullptr; pipe = nullptr;
        report(result == 0, "Script build succeeded", "Script build failed; see compiler diagnostics above. The previous DLL remains active.");
        if (result == 0) scripts.reload(ENGINE_SCRIPT_PATH, scene, registry, context, true);
#endif
    }
};
EditorProject::EditorProject(Scene &s, ComponentRegistry &r, const ComponentCreateContext &c, EditorSelection &sel,
                             EditorPlayState &p, LayerRegistry &l, CameraSystem &cam, ScriptModule &mod)
    : impl(std::make_unique<Impl>(s,r,c,sel,p,l,cam,mod))
{
    impl->sourceFingerprint = impl->scriptFingerprint();

    try {
        const auto config = Json::parse(SceneSerialization::readFile("game.json"));
        const auto path = config.at("startupScene").get<std::string>();
        if (restore(SceneSerialization::readFile(path))) {
            impl->scenePath = path;
            Logger::write(LogLevel::Info, "Editor", "Opened " + path);
        }
    } catch (const std::exception &e) { Logger::write(LogLevel::Warning, "Editor", e.what()); }
}
EditorProject::~EditorProject() = default;
std::string EditorProject::capture()
{
    auto &s = *impl;
    auto j = Json::parse(SceneSerialization::capture(s.scene));
    j["layers"] = Json::object();
    for (int i = 8; i < 32; ++i) j["layers"][std::to_string(i)] = s.layers.getLayerName(i);
    j["cameras"] = Json::array();
    for (int i = 0; i < 3; ++i) j["cameras"].push_back(s.cameras.getPrimaryCameraId(static_cast<CameraOutputTarget>(i)));
    return j.dump(2);
}
bool EditorProject::restore(const std::string &text)
{
    auto &s = *impl;
    try {
        auto j = Json::parse(text);
        LayerRegistry checkedLayers;
        if (j.contains("layers")) for (int i = 8; i < 32; ++i) {
            const auto name = j.at("layers").value(std::to_string(i), std::string());
            if (!name.empty() && !checkedLayers.setLayerName(i, name)) throw std::runtime_error("Invalid or duplicate layer name");
        }
        ObjectId cameraIds[3]{};
        if (j.contains("cameras")) for (int i = 0; i < 3; ++i) cameraIds[i] = j.at("cameras").at(i).get<ObjectId>();
        std::string error;
        if (!SceneSerialization::restore(s.scene, text, s.registry, s.context, error)) throw std::runtime_error(error);
        for (int i = 8; i < 32; ++i) s.layers.clearLayerName(i);
        for (int i = 8; i < 32; ++i) if (!checkedLayers.getLayerName(i).empty()) s.layers.setLayerName(i, checkedLayers.getLayerName(i));
        for (int i = 0; i < 3; ++i) {
            auto output = static_cast<CameraOutputTarget>(i); s.cameras.clearPrimaryCamera(output);
            s.cameras.setPrimaryCamera(s.scene, output, cameraIds[i]);
        }
        s.selection.clear();
        return true;
    } catch (const std::exception &e) { s.report(false, "", e.what()); return false; }
}
void EditorProject::drawFileMenu()
{
    auto &s = *impl;
    ImGui::BeginDisabled(!s.play.isEditing());
    if (ImGui::MenuItem("New Scene")) s.dialog(4, s.scenePath);
    if (ImGui::MenuItem("Open Scene...", "Ctrl+O")) s.dialog(1, s.scenePath);
    if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
        std::string error; s.report(SceneSerialization::writeFile(s.scenePath, capture(), error), "Saved " + s.scenePath, error);
    }
    if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S")) s.dialog(2, s.scenePath);
    ImGui::Separator();
    if (ImGui::MenuItem("Save Selected as Prefab...", nullptr, false, s.selection.getSelectedObject(s.scene) != nullptr)) s.dialog(3, "res/prefabs/object.prefab");
    ImGui::EndDisabled();
    ImGui::Separator();
    if (ImGui::MenuItem("Build Project", "F7", false, !s.building())) s.build();
    if (ImGui::MenuItem("Create C++ Script...", nullptr, false, !s.building())) s.openNewScriptDialog = true;
    if (ImGui::MenuItem("Reload Scripts", nullptr, false, !s.building())) s.scripts.reload(ENGINE_SCRIPT_PATH, s.scene, s.registry, s.context, true);
    ImGui::TextDisabled("Package: tools/package_game.ps1");
}
void EditorProject::buildScripts() { impl->build(); }
bool EditorProject::isBuilding() const { return impl->building(); }
bool EditorProject::createScriptComponent(const std::string &name, std::string &error)
{
    if (name.size() >= sizeof(impl->scriptName)) {
        error = "Script names must be shorter than 128 characters.";
        return false;
    }

    std::snprintf(impl->scriptName, sizeof(impl->scriptName), "%s", name.c_str());
    return impl->createScript(error);
}

bool EditorProject::editScriptComponent(const std::string &typeName, std::string &error)
{
    if (typeName.empty() || !(std::isalpha(static_cast<unsigned char>(typeName.front())) || typeName.front() == '_') ||
        !std::all_of(typeName.begin() + 1, typeName.end(), [](unsigned char character) {
            return std::isalnum(character) || character == '_';
        })) {
        error = "Invalid script component name.";
        return false;
    }

    const fs::path sourceFile = fs::absolute(fs::path("scripts") / (typeName + ".cpp"));
    if (!fs::is_regular_file(sourceFile)) {
        error = "No source file exists for " + typeName + ".";
        return false;
    }

#ifdef _WIN32
    const std::string configuredEditor = EditorPreferences::getInstance().getPreference<std::string>("ExternalEditor");
    const std::wstring sourceArgument = sourceFile.wstring();
    const std::wstring command = configuredEditor.empty()
        ? L"code --reuse-window \"" + sourceArgument + L"\""
        : L"\"" + fs::u8path(configuredEditor).wstring() + L"\" \"" + sourceArgument + L"\"";
    std::vector<wchar_t> mutableCommand(command.begin(), command.end());
    mutableCommand.push_back(L'\0');

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION child{};
    if (!CreateProcessW(nullptr, mutableCommand.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startup, &child)) {
        error = "Could not open the script editor (Windows error " + std::to_string(GetLastError()) + ").";
        return false;
    }

    CloseHandle(child.hThread);
    CloseHandle(child.hProcess);
    Logger::write(LogLevel::Info, "Scripts", "Opened " + sourceFile.generic_u8string());
    return true;
#else
    error = "Opening an external editor is currently supported on Windows only.";
    return false;
#endif
}
void EditorProject::notifyWindowFocusGained() { impl->sourceCheckRequested = true; }
void EditorProject::tick()
{
    impl->pollBuild();

    if (impl->sourceCheckRequested && !impl->building()) {
        impl->sourceCheckRequested = false;
        const std::string currentFingerprint = impl->scriptFingerprint();

        if (currentFingerprint != impl->sourceFingerprint) {
            impl->sourceFingerprint = currentFingerprint;
            Logger::write(LogLevel::Info, "Scripts", "Saved script source detected; rebuilding components.");
            impl->build();
        }
    }

    if (!impl->building()) impl->scripts.reload(ENGINE_SCRIPT_PATH, impl->scene, impl->registry, impl->context);
}
void EditorProject::drawPanels()
{
    auto &s = *impl;

    if (s.openNewScriptDialog) {
        ImGui::OpenPopup("Create C++ Script");
        s.openNewScriptDialog = false;
    }

    if (ImGui::BeginPopupModal("Create C++ Script", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking)) {
        ImGui::TextUnformatted("Creates scripts/<name>.cpp with Setup, PreUpdate, Update and Destroy hooks.");
        ImGui::InputText("Script name", s.scriptName, sizeof(s.scriptName));
        ImGui::TextDisabled("The name becomes the component name under User Scripts.");

        if (ImGui::Button("Create and build")) {
            std::string error;
            if (s.createScript(error)) {
                ImGui::CloseCurrentPopup();
            } else {
                s.report(false, "", error);
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    if (!ImGui::GetIO().WantTextInput) {
        if (ImGui::IsKeyPressed(ImGuiKey_F7) && !s.building()) s.build();
        if (s.play.isEditing() && ImGui::GetIO().KeyCtrl) {
            if (ImGui::IsKeyPressed(ImGuiKey_O)) s.dialog(1, s.scenePath);
            if (ImGui::IsKeyPressed(ImGuiKey_S)) {
                if (ImGui::GetIO().KeyShift) s.dialog(2, s.scenePath);
                else { std::string error; s.report(SceneSerialization::writeFile(s.scenePath, capture(), error), "Saved " + s.scenePath, error); }
            }
        }
    }
    if (s.openDialog)
    {
        ImGui::OpenPopup("Scene / Prefab");
        s.openDialog = false;
    }

    if (ImGui::BeginPopupModal("Scene / Prefab", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking)) 
    {
        if (s.action == 1 || s.action == 4) 
        {
            ImGui::TextWrapped("This replaces the current scene. Save any changes you want to keep first.");
        }
        if (s.action != 4) 
        {
            ImGui::InputText("Path", s.path, sizeof(s.path));
        }
        if (ImGui::Button(s.action == 4 ? "Create empty scene" : s.action == 1 ? "Open" : "Save")) 
        {
            bool ok = false; std::string error;
            try {
                auto path = fs::u8path(s.path);
                if (s.action == 1) 
                    ok = restore(SceneSerialization::readFile(s.path));
                else if (s.action == 2) 
                {
                    if (path.extension() != ".scene") path += ".scene";
                    ok = SceneSerialization::writeFile(path.u8string(), capture(), error);
                } else if (s.action == 3) 
                {
                    if (path.extension() != ".prefab") path += ".prefab";
                    auto *selected = s.selection.getSelectedObject(s.scene);
                    if (selected) ok = SceneSerialization::savePrefab(*selected, path.u8string(), error);
                    else error = "Select an object first";
                } else 
                {
                    ok = restore("{\"format\":\"ricochet.scene\",\"version\":1,\"objects\":[]}"); 
                    s.scenePath = "res/scenes/untitled.scene"; 
                }
                if (ok) 
                {
                    if (s.action == 1 || s.action == 2) s.scenePath = path.u8string();
                    s.report(true, "Scene / prefab operation completed", ""); ImGui::CloseCurrentPopup();
                } else if (!error.empty()) 
                {
                    s.report(false, "", error);
                }
            } catch (const std::exception &e) { s.report(false, "", e.what()); }
        }
        ImGui::SameLine(); if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::Begin("Assets");
    ImGui::TextUnformatted(s.directory.generic_u8string().c_str());
    if (s.directory != fs::path("res") && ImGui::Button("Up")) s.directory = s.directory.parent_path();
    ImGui::InputTextWithHint("##assetfilter", "Filter filenames...", s.filter, sizeof(s.filter));
    std::error_code error;
    std::vector<fs::directory_entry> items;
    for (fs::directory_iterator it(s.directory, error), end; !error && it != end; it.increment(error)) items.push_back(*it);
    if (error) ImGui::TextWrapped("%s", error.message().c_str());
    std::sort(items.begin(), items.end(), [](const auto &a, const auto &b) { return a.path().filename() < b.path().filename(); });
    for (const auto &entry : items) {
        const auto name = entry.path().filename().u8string();
        if (name.find(s.filter) == std::string::npos) continue;
        std::error_code ec;
        if (entry.is_symlink(ec)) continue;
        const bool directory = entry.is_directory(ec);
        const auto path = entry.path().generic_u8string();
        const auto label = (directory ? "[Folder] " : "") + name;
        if (ImGui::Selectable(label.c_str(), s.selectedAsset == path, ImGuiSelectableFlags_AllowDoubleClick)) {
            s.selectedAsset = path;
            if (ImGui::IsMouseDoubleClicked(0)) {
                if (directory) { s.directory = entry.path(); break; }
                if (s.play.isEditing() && entry.path().extension() == ".scene") s.dialog(1, path);
                if (s.play.isEditing() && entry.path().extension() == ".prefab") {
                    std::string failure;
                    auto *object = SceneSerialization::instantiatePrefab(s.scene, path, s.registry, s.context, failure);
                    if (object) s.selection.selectObject(object);
                    s.report(object != nullptr, "Instantiated " + path, failure);
                }
            }
        }
        if (!directory && ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("ASSET_PATH", path.c_str(), path.size() + 1);
            ImGui::TextUnformatted(name.c_str()); ImGui::EndDragDropSource();
        }
    }
    ImGui::Separator();
    if (!s.selectedAsset.empty()) {
        ImGui::TextWrapped("%s", s.selectedAsset.c_str());
        if (ImGui::Button("Copy asset path")) ImGui::SetClipboardText(s.selectedAsset.c_str());
    }
    ImGui::TextDisabled("Double-click scenes to open, prefabs to instantiate.");
    ImGui::End();

    ImGui::Begin("Log");
    if (ImGui::Button("Clear view")) Logger::clearView();
    ImGui::SameLine(); ImGui::Checkbox("Info", &s.info); ImGui::SameLine(); ImGui::Checkbox("Warnings", &s.warnings);
    ImGui::SameLine(); ImGui::Checkbox("Errors", &s.errors); ImGui::SameLine(); ImGui::Checkbox("Follow", &s.follow);
    s.logFilter.Draw("Filter");
    ImGui::TextDisabled("%s", Logger::filePath().c_str());
    if (ImGui::Button("Copy log")) {
        std::string text; for (const auto &e : Logger::entries()) text += e.time + " [" + e.source + "] " + e.message + "\n";
        ImGui::SetClipboardText(text.c_str());
    }

    ImGui::BeginChild("Log entries", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar);

    const bool atBottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 4;

    for (const auto &e : Logger::entries()) {
        if ((e.level == LogLevel::Info && !s.info) || (e.level == LogLevel::Warning && !s.warnings) || (e.level == LogLevel::Error && !s.errors)) continue;
        const auto line = e.time + " [" + e.source + "] " + e.message;
        if (!s.logFilter.PassFilter(line.c_str())) continue;
        ImGui::PushStyleColor(ImGuiCol_Text, e.level == LogLevel::Error ? ImVec4(1,.4f,.4f,1) : e.level == LogLevel::Warning ? ImVec4(1,.8f,.3f,1) : ImVec4(.8f,.85f,.9f,1));
        ImGui::TextUnformatted(line.c_str()); ImGui::PopStyleColor();
    }
    if (s.follow && atBottom) 
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild(); 
    ImGui::End();
}
