#include "objectId.hpp"
#include <atomic>
#include <limits>

namespace
{
std::atomic<ObjectId> nextId{1};
}

ObjectId AcquireObjectId(ObjectId requestedId)
{
    if (requestedId == InvalidObjectId) {
        return nextId.fetch_add(1);
    }

    if (requestedId == std::numeric_limits<ObjectId>::max()) {
        return requestedId;
    }

    const ObjectId nextCandidate = requestedId + 1;
    ObjectId current = nextId.load();

    while (current < nextCandidate && !nextId.compare_exchange_weak(current, nextCandidate)) {
    }

    return requestedId;
}