#pragma once
#include "engineApi.hpp"
#include <cstdint>

using ObjectId = std::uint64_t;

inline constexpr ObjectId InvalidObjectId = 0;

//// Returns a new ID, or reserves a ID during scene loading.
ENGINE_API ObjectId AcquireObjectId(
    ObjectId requestedId = InvalidObjectId
);