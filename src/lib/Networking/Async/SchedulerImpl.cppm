export module Networking:Async.SchedulerImpl;

import :Async.Scheduler;
import std;

inline namespace {
    using Result = typename Scheduler::Result;
}

Scheduler::Scheduler(std::size_t total_workers)
    : m_running{false}
    , m_pool{total_workers}
{}

Result Scheduler::run() {
    using enum Result;
    m_pool.startWorkers();
    m_running = true;
    return SUCCESS;
}

Result Scheduler::waitAll() {
    using enum Result;
    m_pool.joinAll();
    m_running = false;
    return SUCCESS;
}
