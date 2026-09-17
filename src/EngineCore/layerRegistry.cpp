#include "layerRegistry.hpp"

#include <array>

struct LayerRegistry::Impl
{
    std::array<std::string, LayerCount> names;
};

LayerRegistry::LayerRegistry() : impl(std::make_unique<Impl>())
{
    impl->names[0] = "Default";
    impl->names[1] = "TransparentFX";
    impl->names[2] = "Ignore Raycast";
    impl->names[4] = "Water";
    impl->names[5] = "UI";
}

LayerRegistry::~LayerRegistry() = default;

bool LayerRegistry::setLayerName(int layer, const std::string &name)
{
    if (!IsUserLayer(layer) || name.empty()) {
        return false;
    }

    for (int index = 0; index < LayerCount; ++index) {
        if (index != layer && impl->names[index] == name) {
            return false;
        }
    }

    impl->names[layer] = name;
    return true;
}

bool LayerRegistry::clearLayerName(int layer)
{
    if (!IsUserLayer(layer)) {
        return false;
    }

    impl->names[layer].clear();
    return true;
}

const std::string &LayerRegistry::getLayerName(int layer) const
{
    static const std::string emptyName;

    if (!IsValidLayer(layer)) {
        return emptyName;
    }

    return impl->names[layer];
}

int LayerRegistry::findLayer(const std::string &name) const
{
    if (name.empty()) {
        return -1;
    }

    for (int index = 0; index < LayerCount; ++index) {
        if (impl->names[index] == name) {
            return index;
        }
    }

    return -1;
}

LayerMask LayerRegistry::maskForLayerName(const std::string &name) const
{
    const int layer = findLayer(name);
    return layer >= 0 ? LayerBit(layer) : NoLayers;
}