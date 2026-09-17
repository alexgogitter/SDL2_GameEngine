#pragma once

#include <cstdint>
#include <limits>

using LayerMask = std::uint32_t;

inline constexpr int LayerCount = 32;
inline constexpr LayerMask NoLayers = 0;
inline constexpr LayerMask AllLayers = std::numeric_limits<LayerMask>::max();

inline constexpr int FirstUserLayer = 8;

constexpr bool IsUserLayer(int layer) { return layer >= FirstUserLayer && layer < LayerCount; }

constexpr bool IsValidLayer(int layer) { return layer >= 0 && layer < LayerCount; }

constexpr LayerMask LayerBit(int layer) { return IsValidLayer(layer) ? LayerMask{1} << static_cast<unsigned int>(layer) : NoLayers; }

constexpr bool LayerMaskContains(LayerMask mask, int layer) { return (mask & LayerBit(layer)) != 0; }