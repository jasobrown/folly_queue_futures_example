#pragma once

#include <folly/futures/Promise.h>

namespace crdb_pmem {

enum class Action { READ, WRITE };

class PmemEvent {
  public:
    PmemEvent();
    PmemEvent(Action a, int v, folly::Promise<int> p) noexcept;
    PmemEvent& operator=(PmemEvent&& obj) noexcept;
    PmemEvent(PmemEvent& obj) noexcept;  // copy ctor
    PmemEvent(PmemEvent&& obj) noexcept; //move ctor

    Action action_;
    int value_;
    folly::Promise<int> promise_;
};

} // namespace crdb-pmem
