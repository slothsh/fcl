namespace detail {
    using Result = typename ThreadPool::Result;
    using IdType = typename ThreadPool::IdType;
}

std::random_device ThreadPool::s_random_device{};
std::mt19937_64 ThreadPool::s_generator{s_random_device()};
std::uniform_int_distribution<detail::IdType> ThreadPool::s_distribution{};

ThreadPool::ThreadPool(size_t total_workers)
    : m_size{total_workers}
    , m_workers{}
{}

ThreadPool::Result ThreadPool::startWorkers() {
    while (!m_queue.empty()) {
        auto& [id, function] = m_queue.back();
        this->spawnWithId(id, std::move(function));
        m_queue.pop_back();
    }

    return SUCCESS;
}

detail::Result ThreadPool::joinAll() {
    for (auto& [id, worker] : m_workers) {
        worker.join();
    }
    return SUCCESS;
}
