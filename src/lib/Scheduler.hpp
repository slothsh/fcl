#pragma once

#include <concepts>
#include <cstddef>
#include "ThreadPool.hpp"

class Scheduler {
public:
    enum Result : int {
        SUCCESS,
        FAILED,
    };

    Scheduler(size_t total_workers);

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
