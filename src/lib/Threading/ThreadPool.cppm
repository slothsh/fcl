export module Threading:ThreadPool;

import std;

export class ThreadPool {
public:
    enum Result {
        SUCCESS,
        FAILED,
    };

    using IdType = std::size_t;
    using FnType = std::function<Result(IdType)>;
    using PairType = std::pair<IdType, FnType>;
    using WorkerType = std::thread;
    using PoolType = std::unordered_map<IdType, WorkerType>;
    using QueueType = std::deque<PairType>;

    static std::random_device s_random_device;
    static std::mt19937_64 s_generator;
    static std::uniform_int_distribution<IdType> s_distribution;

    ThreadPool(std::size_t total_workers);

    Result startWorkers();
    Result joinAll();

    template<std::invocable<IdType> F>
    inline Result spawn(F&& function) {
        auto const id = s_distribution(s_generator);
        m_workers.try_emplace(id, WorkerType(std::forward<F>(function), id));
        return SUCCESS;
    }

    template<std::invocable<IdType> F>
    inline Result spawnWithId(IdType id, F&& function) {
        m_workers.try_emplace(id, WorkerType(std::forward<F>(function), id));
        return SUCCESS;
    }

    template<std::invocable<IdType> F>
    inline Result enqueue(F&& function) {
        m_queue.emplace_back(s_distribution(s_generator), FnType(std::forward<F>(function)));
        return SUCCESS;
    }

private:
    std::size_t m_size;
    PoolType m_workers;
    QueueType m_queue;
};
