#pragma once

#include <cstdint>
#include <deque>
#include <map>
#include <string>
#include <vector>

#include <glm.hpp>

#include "component.hpp"
#include "material2D.hpp"

class Resource_manager;
class SpriteRendererComponent;

/// Component-local 2D animation state machine driven by string events.
///
/// This controller targets one SpriteRendererComponent. It can compile
/// TexturePacker generic XML atlases where sprite names follow
/// PrefixFrame_Angle, for example Image0001_45. Each angle becomes a separate
/// clip/state named clipBaseName_angle.
class AnimationControllerComponent final : public Component
{
  public:
    /// Creates an animator that writes frames into an existing sprite renderer.
    /// @param parent Non-owned object that owns this component.
    /// @param resources Resource cache used to load atlas textures.
    /// @param targetSprite Sprite component whose material/source rects are animated.
    AnimationControllerComponent(Object *parent, Resource_manager &resources, SpriteRendererComponent &targetSprite);

    /// Loads a TexturePacker generic XML atlas and compiles one clip per angle.
    /// @param xmlPath Path to the `.xml` file.
    /// @param clipBaseName Prefix for generated state names, e.g. Drone_Forward.
    /// @param fps Playback rate for every generated clip.
    /// @param loop Whether generated clips wrap after their final frame.
    /// @return True when at least one clip was compiled.
    bool loadTexturePackerAtlas(const std::string &xmlPath, const std::string &clipBaseName, float fps = 12.0f, bool loop = true);

    /// Selects the initial state and immediately applies its first frame.
    /// @return True when the state exists.
    bool setDefaultState(const std::string &stateName);

    /// Adds a transition consumed only while fromState is current.
    /// @return True when both states exist.
    bool addTransition(const std::string &fromState, const std::string &eventName, const std::string &toState);

    /// Adds a transition consumed from any current state.
    /// @return True when toState exists.
    bool addAnyTransition(const std::string &eventName, const std::string &toState);

    /// Queues a component-local animation event for the next Update().
    void queueEvent(const std::string &eventName);

    /// Immediately switches to a state and applies its current/first frame.
    /// @return True when the state exists.
    bool play(const std::string &stateName, bool restart = true);

    /// @return Current animation state name, or an empty string before playback.
    const std::string &getCurrentState() const { return currentState; }

  private:
    struct AnimationFrame
    {
        TextureHandle albedo = InvalidTextureHandle;
        TextureHandle alphaMask = InvalidTextureHandle;
        TextureHandle diffuse = InvalidTextureHandle;
        TextureHandle normal = InvalidTextureHandle;
        TextureHandle height = InvalidTextureHandle;
        TextureHandle emission = InvalidTextureHandle;
        TextureHandle specular = InvalidTextureHandle;

        glm::vec4 albedoUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
        glm::vec4 alphaMaskUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
        glm::vec4 diffuseUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
        glm::vec4 normalUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
        glm::vec4 heightUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
        glm::vec4 emissionUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
        glm::vec4 specularUvRect = {0.0f, 0.0f, 1.0f, 1.0f};

        glm::vec2 drawSizePixels = {0.0f, 0.0f};
    };

    struct AnimationClip
    {
        std::string name;
        std::vector<AnimationFrame> frames;
        float fps = 12.0f;
        bool loop = true;
    };

    void Update(std::uint64_t deltaTime) override;

    void processQueuedEvents();
    bool tryTransitionForEvent(const std::string &eventName);
    void applyCurrentFrame();
    AnimationClip *getCurrentClip();
    const AnimationClip *getCurrentClip() const;

    Resource_manager *resources = nullptr;
    SpriteRendererComponent *sprite = nullptr;

    std::map<std::string, AnimationClip> clips;
    std::map<std::string, std::map<std::string, std::string>> transitions;
    std::map<std::string, std::string> anyTransitions;
    std::deque<std::string> queuedEvents;

    std::string currentState;
    std::size_t currentFrameIndex = 0;
    float frameAccumulatorSeconds = 0.0f;
};
