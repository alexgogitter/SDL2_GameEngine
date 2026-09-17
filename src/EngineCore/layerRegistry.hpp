#pragma once

#include "engineApi.hpp"
#include "layer.hpp"

#include <memory>
#include <string>

class LayerRegistry
{
  public:
    ENGINE_API LayerRegistry();
    ENGINE_API ~LayerRegistry();

    LayerRegistry(const LayerRegistry &) = delete;
    LayerRegistry &operator=(const LayerRegistry &) = delete;
    LayerRegistry(LayerRegistry &&) = delete;
    LayerRegistry &operator=(LayerRegistry &&) = delete;

    /// Names a user layer from 8 to 31.
    /// Empty or duplicate names are rejected.
    ENGINE_API bool setLayerName(int layer, const std::string &name);

    /// Clears a user-defined layer name.
    ENGINE_API bool clearLayerName(int layer);

    /// Returns an empty string for invalid or unnamed layers.
    ENGINE_API const std::string &getLayerName(int layer) const;

    /// Returns -1 when the name is not registered.
    ENGINE_API int findLayer(const std::string &name) const;

    /// Returns NoLayers when the name is not registered.
    ENGINE_API LayerMask maskForLayerName(const std::string &name) const;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};