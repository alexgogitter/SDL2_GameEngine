#include "window.hpp"
#include "render2D.hpp"
#include "render3D.hpp"
#include "resource_manager.hpp"
#include "scene.hpp"
#include "sceneSerialization.hpp"
#include "componentRegistry.hpp"
#include "builtInComponents.hpp"
#include "scriptComponent.hpp"
#include "scriptModule.hpp"
#include "logger.hpp"
#include "object.hpp"
#include "physxContext.hpp"
#include "physicsWorld3D.hpp"
#include "physicsWorld2D.hpp"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <iostream>
#include <cmath>
#include <stdexcept>
#include <SDL_image.h>

using Json = nlohmann::json;

void check(bool ok, const std::string &message) { if (!ok) throw std::runtime_error(message); }
int setupCount = 0, preCount = 0, updateCount = 0, destroyCount = 0;
class TestScript : public ScriptComponent
{
public:
    float speed = 7.5f;
    std::string asset = "res/texture with spaces.png";
    glm::vec3 direction{1,2,3};
    bool enabled = true;
    int count = 9;
    explicit TestScript(Object *o) : ScriptComponent("TestScript", o) {}
protected:
    void Setup() override { ++setupCount; check(speed == 7.5f, "Setup must see restored properties"); }
    void PreUpdate(std::uint64_t) override { ++preCount; }
    void Update(std::uint64_t) override { ++updateCount; }
    void Destroy() override { ++destroyCount; }
    std::size_t GetPropertyCount() const override { return 5; }
    bool GetProperty(std::size_t i, ComponentProperty &p) override
    {
        switch (i) {
        case 0: p = {"speed", "Speed", ComponentPropertyType::Float, &speed}; return true;
        case 1: p = {"asset", "Asset", ComponentPropertyType::String, &asset}; return true;
        case 2: p = {"direction", "Direction", ComponentPropertyType::Vector3, &direction}; return true;
        case 3: p = {"enabled", "Enabled", ComponentPropertyType::Boolean, &enabled}; return true;
        case 4: p = {"count", "Count", ComponentPropertyType::Integer, &count}; return true;
        default: return false;
        }
    }
};
Component *createTest(Object *o, const ComponentCreateContext &) { return new TestScript(o); }
int main(int, char **)
{
    try {
        Logger::initialize("logs/foundation-tests.log");
        Window window(320,240,0,IMG_INIT_PNG,"Engine foundation tests");
        check(window.Initialize(), "Window initialization");
        SDL_HideWindow(window.GetSDLWindow());
        Renderer2D renderer(window); Renderer3D renderer3D(window);
        check(renderer.Renderer_Init() == 0 && renderer3D.Initialize(), "Renderer initialization");
        {
            PhysXContext physics; PhysicsWorld3D world3D(physics); PhysicsWorld2D world2D(physics);
            Resource_manager resources;
            ScriptModule module;
            ComponentRegistry registry; RegisterBuiltInComponents(registry);
            Scene scene(resources, renderer, renderer3D);
            ComponentCreateContext context{&resources,&world2D,&world3D,&renderer,&renderer3D};
            
            registry.registerType("TestScript", "Test", "Tests", createTest, nullptr, 0);
            auto *root = scene.createObject("Parent"); const auto rootId = root->getId();
            root->transform.setPosition(glm::vec3(3,4,5)); root->setLayer(12);
            auto *child = scene.createChildObject(rootId, "Child"); const auto childId = child->getId();
            registry.createAndAttach("TestScript", *child, context);
            auto snapshot = SceneSerialization::capture(scene);
            check(setupCount == 0, "Setup must not run in edit mode");
            scene.update(16); scene.update(16);
            check(setupCount == 1 && preCount == 2 && updateCount == 2, "Script lifecycle order/count");
            scene.destroyObject(childId); scene.createObject("Runtime spawn");
            std::string error;
            check(SceneSerialization::restore(scene,snapshot,registry,context,error), error);
            check(scene.getRootObjectCount() == 1 && scene.findObject(childId), "Full hierarchy restoration");
            check(scene.findObject(childId)->getParentObject()->getId() == rootId, "Parent restoration");
            check(SceneSerialization::capture(scene) == snapshot, "Properties and transforms round trip");
            auto bad = Json::parse(snapshot); bad["objects"].push_back(bad["objects"][0]);
            check(!SceneSerialization::restore(scene,bad.dump(),registry,context,error), "Duplicate IDs rejected");
            check(SceneSerialization::capture(scene) == snapshot, "Failed load preserves scene");
            bad = Json::parse(snapshot); bad["objects"][0]["children"][0]["components"][0]["type"] = "MissingScript";
            check(!SceneSerialization::restore(scene,bad.dump(),registry,context,error), "Unknown component rejected");
            check(!SceneSerialization::restore(scene,"{broken",registry,context,error), "Malformed JSON rejected");
            auto temp = std::filesystem::temp_directory_path() / ("ricochet-tests-" + std::to_string(SDL_GetTicks64()));
            std::filesystem::create_directories(temp);
            const auto prefab = (temp / "object.prefab").u8string();
            check(SceneSerialization::savePrefab(*scene.findObject(rootId),prefab,error), error);
            auto *copy = SceneSerialization::instantiatePrefab(scene,prefab,registry,context,error);
            check(copy && copy->getId() != rootId && copy->getChild(0)->getId() != childId, "Prefab IDs must be fresh");
            check(copy->getChild(0)->getComponentByTypeName("TestScript"), "Prefab component restored");
            const auto sceneFile = (temp / "test.scene").u8string();
            check(SceneSerialization::save(scene,sceneFile,error),error);
            check(SceneSerialization::save(scene,sceneFile,error),"Atomic overwrite");
            check(SceneSerialization::load(scene,sceneFile,registry,context,error),error);
            scene.clear();
            check(destroyCount > 0,"Destroy invoked");
            check(module.reload(ENGINE_SCRIPT_PATH,scene,registry,context,true),"Initial script module load");
            check(registry.contains("SpinScript") && registry.contains("PlayerBehaviorScript") && registry.contains("InventoryScript"),
                "Every individual user script is registered in the User Scripts component category");
            root = scene.createObject("Scripted"); const auto scriptId = root->getId();
            registry.createAndAttach("SpinScript",*root,context);
            ComponentProperty speed; root->getComponentByTypeName("SpinScript")->getProperty(0,speed);
            *static_cast<float *>(speed.value) = 120.0f;
            scene.update(1000);
            const glm::vec3 expectedForward = glm::angleAxis(glm::radians(120.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec3(0.0f, 0.0f, -1.0f);
            const glm::vec3 actualForward = root->transform.getRotation() * glm::vec3(0.0f, 0.0f, -1.0f);
            check(glm::length(expectedForward - actualForward) < .001f,
                "Script rotation must continue past the Euler 90-degree singularity");
            check(module.reload(ENGINE_SCRIPT_PATH,scene,registry,context,true),"Live module reload");
            root = scene.findObject(scriptId);
            root->getComponentByTypeName("SpinScript")->getProperty(0,speed);
            check(*static_cast<float *>(speed.value) == 120.0f,"Reload preserves script properties");
            snapshot = SceneSerialization::capture(scene);
            const auto broken = (temp / "broken.dll").u8string();
            SceneSerialization::writeFile(broken,"not a DLL",error);
            check(!module.reload(broken,scene,registry,context,true),"Broken DLL rejected");
            check(SceneSerialization::capture(scene) == snapshot,"Failed reload keeps scene");
            check(!module.reload(INCOMPATIBLE_FIXTURE_PATH,scene,registry,context,true),"API mismatch rejected");
            check(SceneSerialization::capture(scene) == snapshot,"API mismatch preserves scene");
            check(module.reload(RELOAD_FIXTURE_PATH,scene,registry,context,true),"Replacement code loads");
            scene.update(1000);
            check(std::abs(scene.findObject(scriptId)->transform.getPosition().x - 120.0f) < .001f,
                "Reload must execute new code with preserved properties despite a different class layout");
            scene.clear();
            // Exercise the optional Rigidbody3D dependency and rendering component factories.
            root = scene.createObject("Physics");
            registry.createAndAttach("Rigidbody3D",*root,context);
            registry.createAndAttach("BoxCollider3D",*root,context);
            registry.createAndAttach("MeshRenderer3D",*root,context);
            snapshot = SceneSerialization::capture(scene);
            check(SceneSerialization::restore(scene,snapshot,registry,context,error),error);
            check(scene.getRootObject(0)->getComponentCount() == 4,"Component dependencies restored");
            scene.clear();
            root = scene.createObject("Sprite");
            auto *sprite = registry.createAndAttach("SpriteRenderer2D",*root,context);
            const auto imageFile = (temp / "atlas.bmp").u8string();
            SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0,4,4,32,SDL_PIXELFORMAT_RGBA32);
            check(surface != nullptr,"Create sprite fixture");
            SDL_FillRect(surface,nullptr,SDL_MapRGBA(surface->format,255,100,50,255));
            check(SDL_SaveBMP(surface,imageFile.c_str()) == 0,"Write sprite fixture"); SDL_FreeSurface(surface);
            ComponentProperty texture; sprite->getProperty(0,texture);
            *static_cast<std::string *>(texture.value) = imageFile; sprite->notifyPropertyChanged(texture.key);
            ComponentProperty tint; sprite->getProperty(14,tint); *static_cast<glm::vec4 *>(tint.value) = glm::vec4(.1f,.2f,.3f,.4f);
            snapshot = SceneSerialization::capture(scene);
            check(SceneSerialization::restore(scene,snapshot,registry,context,error),error);
            check(SceneSerialization::capture(scene) == snapshot,"Sprite texture paths and material round trip");
            const auto atlasFile = (temp / "atlas.xml").u8string();
            SceneSerialization::writeFile(atlasFile,"<TextureAtlas imagePath=\"atlas.bmp\" width=\"4\" height=\"4\"><sprite n=\"Image0001_0\" x=\"0\" y=\"0\" w=\"4\" h=\"4\"/></TextureAtlas>",error);
            auto *animation = registry.createAndAttach("AnimationController2D",*scene.getRootObject(0),context);
            animation->restoreState(Json{{"atlases",Json::array({{{"path",atlasFile},{"prefix","Walk"},{"fps",12.0f},{"loop",true}}})},
                {"transitions",Json::object()},{"anyTransitions",Json::object()},{"currentState","Walk_0"},{"frame",0},{"elapsed",0.0f}}.dump());
            snapshot = SceneSerialization::capture(scene);
            check(SceneSerialization::restore(scene,snapshot,registry,context,error),error);
            check(SceneSerialization::capture(scene) == snapshot,"Animation atlas and state round trip");
            std::filesystem::remove(temp / "atlas.bmp"); std::filesystem::remove(temp / "atlas.xml");
            Logger::write(LogLevel::Error,"Test","Expected diagnostic");
            check(SceneSerialization::readFile("logs/foundation-tests.log").find("Expected diagnostic") != std::string::npos,"Logger flushes to disk");
            // Only remove the specific temporary files created by this test.
            std::filesystem::remove(temp / "object.prefab"); std::filesystem::remove(temp / "test.scene");
            std::filesystem::remove(temp / "broken.dll"); std::filesystem::remove(temp);
        }
        renderer3D.Shutdown(); renderer.Renderer_Close(); window.Shutdown();
        std::cout << "All engine foundation tests passed\n";
        return 0;
    } catch (const std::exception &e) { std::cerr << "FAILED: " << e.what() << '\n'; return 1; }
}
