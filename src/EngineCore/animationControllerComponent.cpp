#include <nlohmann/json.hpp>
#include <stdexcept>
#include <SDL.h>
#include "animationControllerComponent.hpp"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <map>
#include <regex>
#include <sstream>

#include "resource_manager.hpp"
#include "spriteRendererComponent.hpp"

namespace
{
struct ParsedSprite
{
    std::string prefix;
    int frameNumber = 0;
    int angle = 0;
    glm::vec4 pixelRect = {0.0f, 0.0f, 0.0f, 0.0f};
};

struct PartialFrame
{
    bool hasAlbedo = false;
    bool hasAlphaMask = false;
    bool hasDiffuse = false;
    bool hasNormal = false;
    bool hasHeight = false;
    bool hasEmission = false;
    bool hasSpecular = false;

    glm::vec4 albedoUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
    glm::vec4 alphaMaskUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
    glm::vec4 diffuseUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
    glm::vec4 normalUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
    glm::vec4 heightUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
    glm::vec4 emissionUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
    glm::vec4 specularUvRect = {0.0f, 0.0f, 1.0f, 1.0f};

    glm::vec2 drawSizePixels = {0.0f, 0.0f};
};

std::string readWholeFile(const std::string &path)
{
    std::ifstream file(path);
    if (!file) {
        return {};
    }

    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

std::map<std::string, std::string> parseAttributes(const std::string &text)
{
    std::map<std::string, std::string> attributes;
    const std::regex attributePattern(R"attr(([A-Za-z][A-Za-z0-9]*)\s*=\s*"([^"]*)")attr");
    for (auto iterator = std::sregex_iterator(text.begin(), text.end(), attributePattern); iterator != std::sregex_iterator(); ++iterator) {
        attributes[(*iterator)[1].str()] = (*iterator)[2].str();
    }
    return attributes;
}

bool hasAttribute(const std::map<std::string, std::string> &attributes, const std::string &name) { return attributes.find(name) != attributes.end(); }

int readIntAttribute(const std::map<std::string, std::string> &attributes, const std::string &name, int fallback = 0)
{
    const auto found = attributes.find(name);
    if (found == attributes.end()) {
        return fallback;
    }
    return std::stoi(found->second);
}

std::string resolveRelativePath(const std::string &baseFilePath, const std::string &referencedPath)
{
    if (referencedPath.empty()) {
        return referencedPath;
    }

    const bool isWindowsAbsolute = referencedPath.size() > 2 && referencedPath[1] == ':' && (referencedPath[2] == '\\' || referencedPath[2] == '/');
    const bool isRootAbsolute = referencedPath.front() == '\\' || referencedPath.front() == '/';
    if (isWindowsAbsolute || isRootAbsolute) {
        return referencedPath;
    }

    const std::size_t slash = baseFilePath.find_last_of("\\/");
    if (slash == std::string::npos) {
        return referencedPath;
    }
    return baseFilePath.substr(0, slash + 1) + referencedPath;
}

glm::vec4 toNormalizedUvRect(const glm::vec4 &pixelRect, float atlasWidth, float atlasHeight) { return {pixelRect.x / atlasWidth, pixelRect.y / atlasHeight, pixelRect.z / atlasWidth, pixelRect.w / atlasHeight}; }

bool parseSpriteName(const std::string &name, std::string &prefix, int &frameNumber, int &angle)
{
    static const std::regex namePattern(R"(^([A-Za-z]+)([0-9]+)_(-?[0-9]+)$)");
    std::smatch match;
    if (!std::regex_match(name, match, namePattern)) {
        return false;
    }

    prefix = match[1].str();
    frameNumber = std::stoi(match[2].str());
    angle = std::stoi(match[3].str());
    return true;
}

void assignParsedSprite(PartialFrame &frame, const ParsedSprite &sprite, const glm::vec4 &uvRect)
{
    if (sprite.prefix == "Image") {
        frame.hasAlbedo = true;
        frame.albedoUvRect = uvRect;
        frame.drawSizePixels = {sprite.pixelRect.z, sprite.pixelRect.w};
    }
    else if (sprite.prefix == "Alpha") {
        frame.hasAlphaMask = true;
        frame.alphaMaskUvRect = uvRect;
    }
    else if (sprite.prefix == "Diffuse") {
        frame.hasDiffuse = true;
        frame.diffuseUvRect = uvRect;
    }
    else if (sprite.prefix == "Normal") {
        frame.hasNormal = true;
        frame.normalUvRect = uvRect;
    }
    else if (sprite.prefix == "Height" || sprite.prefix == "Mist") {
        frame.hasHeight = true;
        frame.heightUvRect = uvRect;
    }
    else if (sprite.prefix == "Emission" || sprite.prefix == "Emit") {
        frame.hasEmission = true;
        frame.emissionUvRect = uvRect;
    }
    else if (sprite.prefix == "Specular" || sprite.prefix == "Gloss") {
        frame.hasSpecular = true;
        frame.specularUvRect = uvRect;
    }
}
} // namespace

AnimationControllerComponent::AnimationControllerComponent(Object *parentObject, Resource_manager &resourceManager, SpriteRendererComponent &targetSprite) : Component("AnimationController2D", parentObject), resources(&resourceManager), sprite(&targetSprite) {}

bool AnimationControllerComponent::loadTexturePackerAtlas(const std::string &xmlPath, const std::string &clipBaseName, float fps, bool loop)
{
    if (resources == nullptr || sprite == nullptr || clipBaseName.empty()) {
        return false;
    }

    const std::string xml = readWholeFile(xmlPath);
    if (xml.empty()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR: Unable to read TexturePacker XML: %s\n", xmlPath.c_str());
        return false;
    }

    const std::regex atlasPattern(R"(<TextureAtlas\b([^>]*)>)");
    std::smatch atlasMatch;
    if (!std::regex_search(xml, atlasMatch, atlasPattern)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR: TexturePacker XML missing TextureAtlas root.\n");
        return false;
    }

    const auto atlasAttributes = parseAttributes(atlasMatch[1].str());
    const auto imagePathAttribute = atlasAttributes.find("imagePath");
    if (imagePathAttribute == atlasAttributes.end()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR: TexturePacker XML missing imagePath.\n");
        return false;
    }

    const int atlasWidth = readIntAttribute(atlasAttributes, "width");
    const int atlasHeight = readIntAttribute(atlasAttributes, "height");
    if (atlasWidth <= 0 || atlasHeight <= 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR: TexturePacker XML has invalid atlas dimensions.\n");
        return false;
    }

    const std::string imagePath = resolveRelativePath(xmlPath, imagePathAttribute->second);
    const TextureHandle srgbAtlas = resources->loadTexture(imagePath.c_str(), TextureColourSpace::SRGB);
    const TextureHandle linearAtlas = resources->loadTexture(imagePath.c_str(), TextureColourSpace::Linear);
    if (srgbAtlas == InvalidTextureHandle || linearAtlas == InvalidTextureHandle) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR: Unable to load TexturePacker atlas image: %s\n", imagePath.c_str());
        return false;
    }

    std::map<int, std::map<int, PartialFrame>> framesByAngle;
    const std::regex spritePattern(R"(<sprite\b([^>]*)/?>)");
    for (auto iterator = std::sregex_iterator(xml.begin(), xml.end(), spritePattern); iterator != std::sregex_iterator(); ++iterator) {
        const auto attributes = parseAttributes((*iterator)[1].str());
        const auto nameAttribute = attributes.find("n");
        if (nameAttribute == attributes.end()) {
            continue;
        }

        if ((hasAttribute(attributes, "r") && attributes.at("r") == "y") || hasAttribute(attributes, "vertices") || hasAttribute(attributes, "triangles")) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR: TexturePacker sprite %s uses rotation/polygon packing; v1 supports rectangles only.\n", nameAttribute->second.c_str());
            return false;
        }

        ParsedSprite parsed;
        if (!parseSpriteName(nameAttribute->second, parsed.prefix, parsed.frameNumber, parsed.angle)) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "WARNING: Ignoring TexturePacker sprite with unsupported name: %s\n", nameAttribute->second.c_str());
            continue;
        }

        parsed.pixelRect = {static_cast<float>(readIntAttribute(attributes, "x")), static_cast<float>(readIntAttribute(attributes, "y")), static_cast<float>(readIntAttribute(attributes, "w")), static_cast<float>(readIntAttribute(attributes, "h"))};
        if (parsed.pixelRect.z <= 0.0f || parsed.pixelRect.w <= 0.0f) {
            continue;
        }

        const glm::vec4 uvRect = toNormalizedUvRect(parsed.pixelRect, static_cast<float>(atlasWidth), static_cast<float>(atlasHeight));
        assignParsedSprite(framesByAngle[parsed.angle][parsed.frameNumber], parsed, uvRect);
    }

    std::size_t compiledClipCount = 0;
    for (auto &anglePair : framesByAngle) {
        AnimationClip clip;
        clip.name = clipBaseName + "_" + std::to_string(anglePair.first);
        clip.fps = fps > 0.0f ? fps : 12.0f;
        clip.loop = loop;

        for (auto &framePair : anglePair.second) {
            const PartialFrame &partial = framePair.second;
            if (!partial.hasAlbedo) {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "WARNING: Skipping animation frame %d for %s because Image map is missing.\n", framePair.first, clip.name.c_str());
                continue;
            }

            AnimationFrame frame;
            frame.albedo = srgbAtlas;
            frame.alphaMask = partial.hasAlphaMask ? linearAtlas : InvalidTextureHandle;
            frame.diffuse = partial.hasDiffuse ? linearAtlas : InvalidTextureHandle;
            frame.normal = partial.hasNormal ? linearAtlas : InvalidTextureHandle;
            frame.height = partial.hasHeight ? linearAtlas : InvalidTextureHandle;
            frame.emission = partial.hasEmission ? srgbAtlas : InvalidTextureHandle;
            frame.specular = partial.hasSpecular ? linearAtlas : InvalidTextureHandle;

            frame.albedoUvRect = partial.albedoUvRect;
            frame.alphaMaskUvRect = partial.alphaMaskUvRect;
            frame.diffuseUvRect = partial.diffuseUvRect;
            frame.normalUvRect = partial.normalUvRect;
            frame.heightUvRect = partial.heightUvRect;
            frame.emissionUvRect = partial.emissionUvRect;
            frame.specularUvRect = partial.specularUvRect;
            frame.drawSizePixels = partial.drawSizePixels;
            clip.frames.push_back(frame);
        }

        if (!clip.frames.empty()) {
            clips[clip.name] = clip;
            ++compiledClipCount;
        }
    }

    if (compiledClipCount == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR: TexturePacker atlas produced no animation clips.\n");
        return false;
    }

    auto source = std::find_if(atlasSources.begin(), atlasSources.end(), [&](const AtlasSource &entry) { return entry.path == xmlPath && entry.prefix == clipBaseName; });
    if (source == atlasSources.end()) atlasSources.push_back({xmlPath, clipBaseName, fps, loop});
    else *source = {xmlPath, clipBaseName, fps, loop};
    if (currentState.empty()) {
        setDefaultState(clips.begin()->first);
    }
    return true;
}

bool AnimationControllerComponent::setDefaultState(const std::string &stateName) { return play(stateName, true); }

bool AnimationControllerComponent::addTransition(const std::string &fromState, const std::string &eventName, const std::string &toState)
{
    if (clips.find(fromState) == clips.end() || clips.find(toState) == clips.end()) {
        return false;
    }

    transitions[fromState][eventName] = toState;
    return true;
}

bool AnimationControllerComponent::addAnyTransition(const std::string &eventName, const std::string &toState)
{
    if (clips.find(toState) == clips.end()) {
        return false;
    }

    anyTransitions[eventName] = toState;
    return true;
}

void AnimationControllerComponent::queueEvent(const std::string &eventName)
{
    if (!eventName.empty()) {
        queuedEvents.push_back(eventName);
    }
}

bool AnimationControllerComponent::play(const std::string &stateName, bool restart)
{
    const auto found = clips.find(stateName);
    if (found == clips.end() || found->second.frames.empty()) {
        return false;
    }

    const bool stateChanged = currentState != stateName;
    currentState = stateName;
    if (restart || stateChanged) {
        currentFrameIndex = 0;
        frameAccumulatorSeconds = 0.0f;
    }
    else {
        currentFrameIndex = std::min(currentFrameIndex, found->second.frames.size() - 1);
    }
    applyCurrentFrame();
    return true;
}

void AnimationControllerComponent::Update(std::uint64_t deltaTime)
{
    processQueuedEvents();

    AnimationClip *clip = getCurrentClip();
    if (clip == nullptr || clip->frames.empty()) {
        return;
    }

    const float frameDuration = 1.0f / std::max(clip->fps, 0.001f);
    frameAccumulatorSeconds += static_cast<float>(deltaTime) / 1000.0f;

    bool frameChanged = false;
    while (frameAccumulatorSeconds >= frameDuration) {
        frameAccumulatorSeconds -= frameDuration;
        if (clip->loop) {
            currentFrameIndex = (currentFrameIndex + 1) % clip->frames.size();
            frameChanged = true;
        }
        else if (currentFrameIndex + 1 < clip->frames.size()) {
            ++currentFrameIndex;
            frameChanged = true;
        }
        else {
            frameAccumulatorSeconds = 0.0f;
            break;
        }
    }

    if (frameChanged) {
        applyCurrentFrame();
    }
}

void AnimationControllerComponent::processQueuedEvents()
{
    while (!queuedEvents.empty()) {
        const std::string eventName = queuedEvents.front();
        queuedEvents.pop_front();
        tryTransitionForEvent(eventName);
    }
}

bool AnimationControllerComponent::tryTransitionForEvent(const std::string &eventName)
{
    const auto stateTransitions = transitions.find(currentState);
    if (stateTransitions != transitions.end()) {
        const auto transition = stateTransitions->second.find(eventName);
        if (transition != stateTransitions->second.end()) {
            return play(transition->second, true);
        }
    }

    const auto anyTransition = anyTransitions.find(eventName);
    if (anyTransition != anyTransitions.end()) {
        return play(anyTransition->second, true);
    }

    if (clips.find(eventName) != clips.end()) {
        return play(eventName, true);
    }

    return false;
}

void AnimationControllerComponent::applyCurrentFrame()
{
    AnimationClip *clip = getCurrentClip();
    if (clip == nullptr || sprite == nullptr || clip->frames.empty()) {
        return;
    }

    currentFrameIndex = std::min(currentFrameIndex, clip->frames.size() - 1);
    const AnimationFrame &frame = clip->frames[currentFrameIndex];

    Material2D &material = sprite->getMaterial();
    material.albedo = frame.albedo;
    material.alphaMask = frame.alphaMask;
    material.diffuse = frame.diffuse;
    material.normal = frame.normal;
    material.height = frame.height;
    material.emission = frame.emission;
    material.specular = frame.specular;

    material.albedoUvRect = frame.albedoUvRect;
    material.alphaMaskUvRect = frame.alphaMaskUvRect;
    material.diffuseUvRect = frame.diffuseUvRect;
    material.normalUvRect = frame.normalUvRect;
    material.heightUvRect = frame.heightUvRect;
    material.emissionUvRect = frame.emissionUvRect;
    material.specularUvRect = frame.specularUvRect;

    if (frame.drawSizePixels.x > 0.0f && frame.drawSizePixels.y > 0.0f) {
        sprite->setDrawSize(frame.drawSizePixels);
    }
}

AnimationControllerComponent::AnimationClip *AnimationControllerComponent::getCurrentClip()
{
    const auto found = clips.find(currentState);
    return found != clips.end() ? &found->second : nullptr;
}

const AnimationControllerComponent::AnimationClip *AnimationControllerComponent::getCurrentClip() const
{
    const auto found = clips.find(currentState);
    return found != clips.end() ? &found->second : nullptr;
}

std::string AnimationControllerComponent::CaptureState() const
{
    nlohmann::json sources = nlohmann::json::array();
    for (const auto &s : atlasSources) sources.push_back({{"path",s.path},{"prefix",s.prefix},{"fps",s.fps},{"loop",s.loop}});
    return nlohmann::json{{"atlases",sources},{"transitions",transitions},{"anyTransitions",anyTransitions},
        {"currentState",currentState},{"frame",currentFrameIndex},{"elapsed",frameAccumulatorSeconds}}.dump();
}
void AnimationControllerComponent::RestoreState(const std::string &text)
{
    const auto data = nlohmann::json::parse(text);
    clips.clear(); atlasSources.clear(); transitions.clear(); anyTransitions.clear(); queuedEvents.clear(); currentState.clear();
    for (const auto &s : data.at("atlases")) {
        if (!loadTexturePackerAtlas(s.at("path"),s.at("prefix"),s.at("fps"),s.at("loop")))
            throw std::runtime_error("Cannot restore animation atlas: " + s.at("path").get<std::string>());
    }
    const auto restoredTransitions = data.at("transitions").get<std::map<std::string, std::map<std::string,std::string>>>();
    for (const auto &from : restoredTransitions) for (const auto &event : from.second)
        if (!addTransition(from.first,event.first,event.second)) throw std::runtime_error("Invalid animation transition");
    for (const auto &event : data.at("anyTransitions").get<std::map<std::string,std::string>>())
        if (!addAnyTransition(event.first,event.second)) throw std::runtime_error("Invalid animation transition");
    const auto state = data.at("currentState").get<std::string>();
    if (!state.empty()) {
        if (!play(state)) throw std::runtime_error("Invalid animation state: " + state);
        currentFrameIndex = std::min(data.at("frame").get<std::size_t>(), getCurrentClip()->frames.size()-1);
        frameAccumulatorSeconds = data.at("elapsed").get<float>();
        if (!std::isfinite(frameAccumulatorSeconds) || frameAccumulatorSeconds < 0) frameAccumulatorSeconds = 0;
        applyCurrentFrame();
    }
}
