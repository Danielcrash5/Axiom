/**
 * @file axiom/src/threading/WorkerPool.cpp
 * @brief Implements the worker pool module.
 */

#include <axiom/threading/WorkerPool.h>

namespace axiom {

    WorkerPool::WorkerPool(size_t threadCount) {
        m_threads.reserve(threadCount);
        for (size_t i = 0; i < threadCount; ++i) {
            m_threads.emplace_back([this] {
                while (true) {
                    std::function<void()> job;
                    {
                        std::unique_lock lock(m_mutex);
                        m_condition.wait(lock, [this] {
                            return m_stop || !m_jobs.empty();
                        });
                        if (m_stop && m_jobs.empty()) return;
                        job = std::move(m_jobs.front());
                        m_jobs.pop();
                    }
                    job();
                }
            });
        }
    }

    WorkerPool::~WorkerPool() {
        {
            std::lock_guard lock(m_mutex);
            m_stop = true;
        }
        m_condition.notify_all();
        for (auto &thread : m_threads) {
            if (thread.joinable()) thread.join();
        }
    }

    void WorkerPool::enqueue(std::function<void()> job) {
        {
            std::lock_guard lock(m_mutex);
            m_jobs.push(std::move(job));
        }
        m_condition.notify_one();
    }

} // namespace axiom
