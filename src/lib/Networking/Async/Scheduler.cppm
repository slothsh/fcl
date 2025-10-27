export module Networking:Async.Scheduler;

import Threading;
import std;

export class Scheduler {
public:
    enum Result : int {
        SUCCESS,
        FAILED,
    };

    Scheduler(std::size_t total_workers);

    template<typename F>
    inline Result enqueue(F&& function) {
        m_pool.spawn(std::forward<F>(function));
        return SUCCESS;
    }

    Result run();
    Result waitAll();

private:
    bool m_should_exit;
    bool m_running;
    ThreadPool m_pool;
};
