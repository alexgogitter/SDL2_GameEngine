#include <SDL.h>
#include "componentRegistry.hpp"

#include "component.hpp"
#include "object.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace
{
struct RegistryEntry
{
    std::string typeName;
    std::string displayName;
    std::string category;
    ComponentFactory factory = nullptr;
    std::vector<std::string> requiredTypeNames;
};
} // namespace

struct ComponentRegistry::Impl
{
    std::vector<std::unique_ptr<RegistryEntry>> entries;

    RegistryEntry *find(const char *typeName)
    {
        if (typeName == nullptr) {
            return nullptr;
        }

        for (const auto &entry : entries) {
            if (entry->typeName == typeName) {
                return entry.get();
            }
        }

        return nullptr;
    }

    const RegistryEntry *find(const char *typeName) const
    {
        if (typeName == nullptr) {
            return nullptr;
        }

        for (const auto &entry : entries) {
            if (entry->typeName == typeName) {
                return entry.get();
            }
        }

        return nullptr;
    }
};

ComponentRegistry::ComponentRegistry() : impl(std::make_unique<Impl>()) {}

ComponentRegistry::~ComponentRegistry() = default;
void ComponentRegistry::swap(ComponentRegistry &other) { impl.swap(other.impl); }

bool ComponentRegistry::registerType(const char *typeName, const char *displayName, const char *category, ComponentFactory factory, const char *const *requiredTypeNames, std::size_t requiredTypeCount)
{
    if (typeName == nullptr || typeName[0] == '\0' || factory == nullptr || impl->find(typeName) != nullptr || (requiredTypeCount > 0 && requiredTypeNames == nullptr)) {
        return false;
    }

    auto entry = std::make_unique<RegistryEntry>();

    entry->typeName = typeName;

    entry->displayName = displayName != nullptr && displayName[0] != '\0' ? displayName : typeName;

    entry->category = category != nullptr && category[0] != '\0' ? category : "Other";

    entry->factory = factory;

    for (std::size_t index = 0; index < requiredTypeCount; ++index) {
        const char *requiredType = requiredTypeNames[index];

        if (requiredType == nullptr || requiredType[0] == '\0' || entry->typeName == requiredType) {
            return false;
        }

        const auto duplicate = std::find(entry->requiredTypeNames.begin(), entry->requiredTypeNames.end(), requiredType);

        if (duplicate != entry->requiredTypeNames.end()) {
            return false;
        }

        entry->requiredTypeNames.emplace_back(requiredType);
    }

    impl->entries.push_back(std::move(entry));

    return true;
}

bool ComponentRegistry::unregisterType(const char *typeName)
{
    if (typeName == nullptr) {
        return false;
    }

    for (const auto &entry : impl->entries) {
        const auto dependency = std::find(entry->requiredTypeNames.begin(), entry->requiredTypeNames.end(), typeName);

        if (dependency != entry->requiredTypeNames.end()) {
            return false;
        }
    }

    const std::size_t previousSize = impl->entries.size();

    impl->entries.erase(std::remove_if(impl->entries.begin(), impl->entries.end(), [typeName](const auto &entry) { return entry->typeName == typeName; }), impl->entries.end());

    return impl->entries.size() != previousSize;
}

bool ComponentRegistry::contains(const char *typeName) const { return impl->find(typeName) != nullptr; }

std::size_t ComponentRegistry::getTypeCount() const { return impl->entries.size(); }

const char *ComponentRegistry::getTypeNameAt(std::size_t index) const
{
    if (index >= impl->entries.size()) {
        return "";
    }

    return impl->entries[index]->typeName.c_str();
}

const char *ComponentRegistry::getDisplayName(const char *typeName) const
{
    const RegistryEntry *entry = impl->find(typeName);
    return entry != nullptr ? entry->displayName.c_str() : "";
}

const char *ComponentRegistry::getCategory(const char *typeName) const
{
    const RegistryEntry *entry = impl->find(typeName);
    return entry != nullptr ? entry->category.c_str() : "";
}

std::size_t ComponentRegistry::getRequiredTypeCount(const char *typeName) const
{
    const RegistryEntry *entry = impl->find(typeName);

    return entry != nullptr ? entry->requiredTypeNames.size() : 0;
}

const char *ComponentRegistry::getRequiredTypeNameAt(const char *typeName, std::size_t index) const
{
    const RegistryEntry *entry = impl->find(typeName);

    if (entry == nullptr || index >= entry->requiredTypeNames.size()) {
        return "";
    }

    return entry->requiredTypeNames[index].c_str();
}

const char *ComponentRegistry::getRemovalBlocker(const char *typeName, const Object &parent) const
{
    if (typeName == nullptr) {
        return "";
    }

    for (const auto &entry : impl->entries) {
        if (parent.getComponentByTypeName(entry->typeName) == nullptr) {
            continue;
        }

        const auto required = std::find(entry->requiredTypeNames.begin(), entry->requiredTypeNames.end(), typeName);

        if (required != entry->requiredTypeNames.end()) {
            return entry->typeName.c_str();
        }
    }

    return "";
}

bool ComponentRegistry::removeFromObject(const char *typeName, Object &parent) const
{
    if (typeName == nullptr || typeName[0] == '\0' || getRemovalBlocker(typeName, parent)[0] != '\0') {
        return false;
    }

    return parent.remove_Component(typeName) == 0;
}

Component *ComponentRegistry::createAndAttach(const char *typeName, Object &parent, const ComponentCreateContext &context) const
{
    const RegistryEntry *rootEntry = impl->find(typeName);


    if (rootEntry == nullptr || parent.getComponentByTypeName(rootEntry->typeName) != nullptr) {
        return nullptr;
    }

    std::vector<std::string> creationStack;
    std::vector<Component *> newlyAttached;

    std::function<Component *(const RegistryEntry *)> attachEntry;

    attachEntry = [&](const RegistryEntry *entry) -> Component * {
        if (entry == nullptr) {
            return nullptr;
        }

        if (Component *existing = parent.getComponentByTypeName(entry->typeName)) {
            return existing;
        }

        if (std::find(creationStack.begin(), creationStack.end(), entry->typeName) != creationStack.end()) {
            return nullptr;
        }

        creationStack.push_back(entry->typeName);

        for (const std::string &requiredType : entry->requiredTypeNames) {
            const RegistryEntry *dependency = impl->find(requiredType.c_str());

            if (dependency == nullptr || attachEntry(dependency) == nullptr) {
                creationStack.pop_back();
                return nullptr;
            }
        }

        std::unique_ptr<Component> pending(entry->factory(&parent, context));
        Component *component = pending.get();

        if (component == nullptr) {
            creationStack.pop_back();
            return nullptr;
        }

        if (component->getParent() != &parent || component->getTypeName() != entry->typeName) {
            creationStack.pop_back();
            return nullptr;
        }

        Component *attached = parent.add_Component(pending.release());

        if (attached == nullptr) {
            creationStack.pop_back();
            return nullptr;
        }

        newlyAttached.push_back(attached);
        creationStack.pop_back();

        return attached;
    };

    Component *result = attachEntry(rootEntry);

    if (result == nullptr) {
        for (auto iterator = newlyAttached.rbegin(); iterator != newlyAttached.rend(); ++iterator) {
            parent.remove_Component(*iterator);
        }
    }

    return result;
}
