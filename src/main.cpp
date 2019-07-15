#include <vector>
#include <thread>
#include <future>
#include <numeric>
#include <iostream>
#include <chrono>

#include <hwloc.h>

#include <folly/MPMCQueue.h>
#include <folly/futures/Future.h>
#include <folly/futures/Future-inl.h>
#include <folly/futures/Promise.h>
#include <folly/portability/Config.h>

#include "pmem_event.h"

void consume(folly::MPMCQueue<crdb_pmem::PmemEvent>& queue)
{
    while (true)
    {
        crdb_pmem::PmemEvent event;
        queue.blockingRead(event);
        event.promise_.setValue(event.value_);
        if (event.value_ == -1)
        {
            std::cout << "received kill sentinel" << std::endl;
            break;
        }
        std::cout << event.value_ << std::endl;
    }
}

void setAffinity(int cpuIndex) {
    cpu_set_t cs;
    CPU_ZERO(&cs);
    CPU_SET(cpuIndex, &cs);
    auto r = pthread_setaffinity_np(pthread_self(), sizeof(cs), &cs);
    if (r != 0) {
        std::cout << "failed to assign thread affinity for cpuIndex " << cpuIndex << ", ret code = " << r << std::endl;
    }
}

void produce(folly::MPMCQueue<crdb_pmem::PmemEvent>& queue, int offset, int cpuIndex)
{
    setAffinity(cpuIndex);
    for (size_t i = 0; i < 1000; ++i)
    {
        auto a = i % 2 == 0 ? crdb_pmem::Action::WRITE : crdb_pmem::Action::READ;
        auto promiseFuture = folly::makePromiseContract<int>();
        int val = offset + i;
        crdb_pmem::PmemEvent event;
        event.action_ = a;
        event.value_ = val;
        event.promise_ = std::move(std::get<0>(promiseFuture));

        bool pushed = false;
        int64_t offset = 100;
        for (int i = 0; i < 8; ++i)
        {
            if (queue.write(event))
            {
                // TODO(jeb) it's not entirely clear to me why I need
                // to move the future...
                int retVal = std::move(std::get<1>(promiseFuture)).get(std::chrono::seconds(1));
                if (retVal != val) {
                    std::cout << "retVal ( " << retVal << ") is different form input val (" << val << ")\n";
                }
                pushed = true;
                break;
            }

            std::this_thread::sleep_for(std::chrono::microseconds((i * offset) + offset));
        }

        if (!pushed)
            std::cout << "failed to push to queue" << std::endl;
    }
}

int cpuCount() {
    hwloc_topology_t topology;
    hwloc_topology_init(&topology);
    hwloc_topology_load(topology);
    int cpuCount = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_PU);
    hwloc_topology_destroy(topology);
    return cpuCount;
}

int main()
{
    folly::MPMCQueue<crdb_pmem::PmemEvent>folly_queue (128);
    std::thread consumer_thread(consume, std::ref(folly_queue));

    std::vector<std::thread*> producers;
    int producer_count = cpuCount();
    for (size_t i = 0; i < producer_count; ++i) {
        std::thread* producer = new std::thread(produce, std::ref(folly_queue), i * 10000, i);
        producers.push_back(producer);
    }
    std::for_each(producers.begin(), producers.end(), [](std::thread* t) { t->join();  });

    // close out the consumer
    auto promiseFuture = folly::makePromiseContract<int>();

    // TODO(jeb) so this worked where I hand-assign each member,
    // but i'd prefer a ctor instead ....
    crdb_pmem::PmemEvent event;
    event.action_ = crdb_pmem::Action::WRITE;
    event.value_ = -1;
    event.promise_ = std::move(std::get<0>(promiseFuture));

    folly_queue.blockingWrite(event);

    std::move(std::get<1>(promiseFuture)).get(std::chrono::seconds(1));
    consumer_thread.join();
    return 0;
}
