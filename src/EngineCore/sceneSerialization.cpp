#include "sceneSerialization.hpp"
#include "scene.hpp"
#include "object.hpp"
#include "componentRegistry.hpp"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <cmath>
#include <limits>
#include <set>
#include <stdexcept>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace
{
using Json = nlohmann::json;
void require(bool condition, const std::string &message)
{
    if (!condition) throw std::runtime_error(message);
}
template<int N> Json vectorJson(const glm::vec<N, float> &v)
{
    Json result = Json::array();
    for (int i = 0; i < N; ++i) result.push_back(v[i]);
    return result;
}
float number(const Json &j)
{
    require(j.is_number(), "Expected a numeric value");
    const float value = j.get<float>();
    require(std::isfinite(value), "Non-finite numeric value");
    return value;
}
template<int N> glm::vec<N, float> readVector(const Json &j)
{
    require(j.is_array() && j.size() == N, "Invalid vector size");
    glm::vec<N, float> result;
    for (int i = 0; i < N; ++i) result[i] = number(j.at(i));
    return result;
}
Json propertyValue(const ComponentProperty &p)
{
    switch (p.type) {
    case ComponentPropertyType::Boolean: return *static_cast<bool *>(p.value);
    case ComponentPropertyType::Integer:
    case ComponentPropertyType::Enumeration: return *static_cast<int *>(p.value);
    case ComponentPropertyType::Float: return *static_cast<float *>(p.value);
    case ComponentPropertyType::Vector2: return vectorJson(*static_cast<glm::vec2 *>(p.value));
    case ComponentPropertyType::Vector3:
    case ComponentPropertyType::Colour3: return vectorJson(*static_cast<glm::vec3 *>(p.value));
    case ComponentPropertyType::Vector4:
    case ComponentPropertyType::Colour4: return vectorJson(*static_cast<glm::vec4 *>(p.value));
    case ComponentPropertyType::String: return *static_cast<std::string *>(p.value);
    }
    throw std::runtime_error("Unknown property type");
}
void applyProperty(ComponentProperty &p, const Json &j)
{
    switch (p.type) {
    case ComponentPropertyType::Boolean:
        require(j.is_boolean(), "Expected boolean"); *static_cast<bool *>(p.value) = j.get<bool>(); break;
    case ComponentPropertyType::Integer:
    case ComponentPropertyType::Enumeration: {
        require(j.is_number_integer(), "Expected integer");
        const auto value = j.get<std::int64_t>();
        require(value >= std::numeric_limits<int>::min() && value <= std::numeric_limits<int>::max(), "Integer out of range");
        require(p.type != ComponentPropertyType::Enumeration || p.enumerationCount == 0 || (value >= 0 && value < p.enumerationCount), "Invalid enumeration value");
        *static_cast<int *>(p.value) = static_cast<int>(value); break;
    }
    case ComponentPropertyType::Float: *static_cast<float *>(p.value) = number(j); break;
    case ComponentPropertyType::Vector2: *static_cast<glm::vec2 *>(p.value) = readVector<2>(j); break;
    case ComponentPropertyType::Vector3:
    case ComponentPropertyType::Colour3: *static_cast<glm::vec3 *>(p.value) = readVector<3>(j); break;
    case ComponentPropertyType::Vector4:
    case ComponentPropertyType::Colour4: *static_cast<glm::vec4 *>(p.value) = readVector<4>(j); break;
    case ComponentPropertyType::String: *static_cast<std::string *>(p.value) = j.get<std::string>(); break;
    }
}
Json captureObject(const Object &o)
{
    const auto &q = o.transform.getRotation();
    Json j = {{"id", o.getId()}, {"name", o.getName()}, {"active", o.isActive()}, {"layer", o.getLayer()},
              {"position", vectorJson(o.transform.getPosition())}, {"rotation", {q.w,q.x,q.y,q.z}},
              {"scale", vectorJson(o.transform.getScale())}, {"colour", vectorJson(o.draw_colour)},
              {"components", Json::array()}, {"children", Json::array()}};
    for (std::size_t i = 0; i < o.getComponentCount(); ++i) {
        auto &c = *o.getComponentAt(i);
        Json properties = Json::object();
        for (std::size_t k = 0; k < c.getPropertyCount(); ++k) {
            ComponentProperty p;
            if (c.getProperty(k, p) && p.serializable && !p.readOnly)
                properties[p.key] = {{"type", static_cast<int>(p.type)}, {"value", propertyValue(p)}};
        }
        j["components"].push_back({{"type", c.getTypeName()}, {"enabled", c.isEnabled()}, {"properties", properties}, {"state", c.captureState()}});
    }
    for (std::size_t i = 0; i < o.getChildCount(); ++i) j["children"].push_back(captureObject(*o.getChild(i)));
    return j;
}
Object *restoreObject(Scene &scene, const Json &j, ObjectId parent, bool freshIds,
                      const ComponentRegistry &registry, const ComponentCreateContext &context, int depth = 0)
{
    require(depth < 256, "Object hierarchy is too deep");
    require(j.at("id").is_number_unsigned() || (j.at("id").is_number_integer() && j.at("id").get<std::int64_t>() > 0), "Invalid object ID");
    const auto storedId = j.at("id").get<ObjectId>();
    require(storedId != InvalidObjectId && storedId < std::numeric_limits<ObjectId>::max(), "Invalid object ID");
    const ObjectId id = freshIds ? InvalidObjectId : storedId;
    auto *o = parent == InvalidObjectId ? scene.createObject(j.at("name").get<std::string>(), id)
                                       : scene.createChildObject(parent, j.at("name").get<std::string>(), id);
    require(o != nullptr, "Duplicate object ID");
    o->setActive(j.at("active").get<bool>());
    require(o->setLayer(j.at("layer").get<int>()), "Invalid layer");
    o->transform.setPosition(readVector<3>(j.at("position")));
    const auto q = readVector<4>(j.at("rotation"));
    require(glm::length(q) > 0.000001f, "Invalid rotation");
    o->transform.setRotation(glm::quat(q.x, q.y, q.z, q.w));
    o->transform.setScale(readVector<3>(j.at("scale")));
    o->draw_colour = readVector<4>(j.at("colour"));
    const auto &components = j.at("components");
    require(components.is_array(), "Expected component array");
    std::set<std::string> types;
    for (const auto &c : components) {
        const auto type = c.at("type").get<std::string>();
        require(types.insert(type).second, "Duplicate component: " + type);
        require(registry.contains(type.c_str()), "Unknown component: " + type);
    }
    // Rigidbody3D is an optional collider dependency; create it before colliders.
    if (types.count("Rigidbody3D"))
        require(registry.createAndAttach("Rigidbody3D", *o, context) != nullptr, "Cannot create Rigidbody3D");
    for (const auto &c : components) {
        const auto type = c.at("type").get<std::string>();
        if (!o->getComponentByTypeName(type))
            require(registry.createAndAttach(type.c_str(), *o, context) != nullptr, "Cannot create component: " + type);
    }
    for (const auto &c : components) {
        auto *component = o->getComponentByTypeName(c.at("type").get<std::string>());
        const auto &properties = c.at("properties");
        require(properties.is_object(), "Expected property object");
        for (std::size_t k = 0; k < component->getPropertyCount(); ++k) {
            ComponentProperty p;
            if (!component->getProperty(k, p) || !p.serializable || p.readOnly || !properties.contains(p.key)) continue;
            const auto &stored = properties.at(p.key);
            require(stored.at("type").get<int>() == static_cast<int>(p.type), "Property type changed: " + std::string(p.key));
            applyProperty(p, stored.at("value"));
        }
        // All fields must be restored before callbacks rebuild resources/physics state.
        for (std::size_t k = 0; k < component->getPropertyCount(); ++k) {
            ComponentProperty p;
            if (component->getProperty(k, p) && p.serializable && !p.readOnly && properties.contains(p.key))
            {
                applyProperty(p, properties.at(p.key).at("value"));
                component->notifyPropertyChanged(p.key);
            }
        }
        component->setEnabled(c.at("enabled").get<bool>());
    }
    for (const auto &c : components) {
        if (c.contains("state") && !c.at("state").get<std::string>().empty())
            o->getComponentByTypeName(c.at("type").get<std::string>())->restoreState(c.at("state").get<std::string>());
    }
    require(j.at("children").is_array(), "Expected children array");
    for (const auto &child : j.at("children")) restoreObject(scene, child, o->getId(), freshIds, registry, context, depth + 1);
    return o;
}
Json document(const std::string &text, const char *kind)
{
    auto j = Json::parse(text);
    require(j.at("format") == kind && j.at("version") == 1, "Unsupported file format or version");
    return j;
}
void checkContext(const ComponentCreateContext &c)
{
    require(c.resources && c.renderer && c.renderer3D, "Missing scene services");
}
}
namespace SceneSerialization
{
std::string capture(const Scene &scene)
{
    Json j = {{"format", "ricochet.scene"}, {"version", 1}, {"objects", Json::array()}};
    for (std::size_t i = 0; i < scene.getRootObjectCount(); ++i) j["objects"].push_back(captureObject(*scene.getRootObject(i)));
    return j.dump(2);
}
bool restore(Scene &scene, const std::string &text, const ComponentRegistry &registry, const ComponentCreateContext &context, std::string &error)
{
    try {
        checkContext(context);
        const auto j = document(text, "ricochet.scene");
        require(j.at("objects").is_array(), "Expected objects array");
        Scene staged(*context.resources, *context.renderer, *context.renderer3D);
        for (const auto &o : j.at("objects")) restoreObject(staged, o, InvalidObjectId, false, registry, context);
        scene.swapContents(staged);
        error.clear();
        return true;
    } catch (const std::exception &e) { error = e.what(); return false; }
}
std::string readFile(const std::string &path)
{
    std::ifstream file(std::filesystem::u8path(path), std::ios::binary);
    require(file.good(), "Cannot open " + path);
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}
bool writeFile(const std::string &path, const std::string &text, std::string &error)
{
    auto target = std::filesystem::u8path(path);
    auto temp = target;
    temp += ".tmp";
    try {
        if (target.has_parent_path()) std::filesystem::create_directories(target.parent_path());
        { std::ofstream file(temp, std::ios::binary | std::ios::trunc);
          file << text;
          file.close();
          require(!file.fail(), "Cannot write " + path); }
#ifdef _WIN32
        require(MoveFileExW(temp.c_str(), target.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0, "Cannot replace " + path);
#else
        std::filesystem::rename(temp, target);
#endif
        error.clear(); return true;
    } catch (const std::exception &e) {
        error = e.what(); std::error_code ignored; std::filesystem::remove(temp, ignored); return false;
    }
}
bool save(const Scene &scene, const std::string &path, std::string &error)
{
    try { return writeFile(path, capture(scene), error); }
    catch (const std::exception &e) { error = e.what(); return false; }
}
bool load(Scene &scene, const std::string &path, const ComponentRegistry &registry, const ComponentCreateContext &context, std::string &error)
{
    try { return restore(scene, readFile(path), registry, context, error); }
    catch (const std::exception &e) { error = e.what(); return false; }
}
bool savePrefab(const Object &object, const std::string &path, std::string &error)
{
    try { return writeFile(path, Json{{"format", "ricochet.prefab"}, {"version", 1}, {"object", captureObject(object)}}.dump(2), error); }
    catch (const std::exception &e) { error = e.what(); return false; }
}
Object *instantiatePrefab(Scene &scene, const std::string &path, const ComponentRegistry &registry, const ComponentCreateContext &context, std::string &error)
{
    ObjectId newRoot = InvalidObjectId;
    try {
        checkContext(context);
        const auto j = document(readFile(path), "ricochet.prefab");
        Scene staged(*context.resources, *context.renderer, *context.renderer3D);
        auto *result = restoreObject(staged, j.at("object"), InvalidObjectId, true, registry, context);
        newRoot = result->getId();
        // Transfer the validated hierarchy without reconstructing component instances.
        scene.appendContents(staged);
        error.clear(); return scene.findObject(newRoot);
    } catch (const std::exception &e) { error = e.what(); return nullptr; }
}
}
