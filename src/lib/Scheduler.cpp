namespace detail {
    using Result = typename Scheduler::Result;
}

Scheduler::Scheduler(size_t total_workers)
    : m_running{false}
    , m_pool{total_workers}
{}

detail::Result Scheduler::run() {
    using enum detail::Result;
    m_pool.startWorkers();
    m_running = true;
    return SUCCESS;
}

detail::Result Scheduler::waitAll() {
    using enum detail::Result;
    m_pool.joinAll();
    m_running = false;
    return SUCCESS;
}
