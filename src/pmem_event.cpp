#include "pmem_event.h"

namespace crdb_pmem{

PmemEvent::PmemEvent() {
}

PmemEvent::PmemEvent(Action a, int v, folly::Promise<int> p) noexcept
{
    action_ = a;
    value_ = v;
    promise_ = std::move(p);
}

// move assignment operator
PmemEvent& PmemEvent::operator=(PmemEvent&& other) noexcept {
    action_ = other.action_;
    value_ = other.value_;
    promise_ = std::move(other.promise_);
    return *this;
}

// copy ctor
PmemEvent::PmemEvent(PmemEvent& other) noexcept {
    action_ = other.action_;
    value_ = other.value_;
    // TODO(jeb) i think this is wrong (performing a move
    // in a copy ctor). it works for this use case, but
    // not sure in general
    promise_ = std::move(other.promise_);
}

// move ctor
PmemEvent::PmemEvent(PmemEvent&& other) noexcept {
    action_ = other.action_;
    value_ = other.value_;
    promise_ = std::move(other.promise_);
}

} // namespace crdb_pmem
