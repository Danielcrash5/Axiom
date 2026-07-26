#include <axiom/threading/WorkerPool.h>

namespace axiom {

    WorkerPool::WorkerPool(size_t threadCount) {
        m_Threads.reserve(threadCount);
        for (size_t i = 0; i < threadCount; ++i) {
            m_Threads.emplace_back([this] {
                while (true) {
                    std::function<void()> job;
                    {
                        std::unique_lock lock(m_Mutex);
                        m_Condition.wait(lock, [this] {
                            return m_Stop || !m_Jobs.empty();
                        });
                        if (m_Stop && m_Jobs.empty()) return;
                        job = std::move(m_Jobs.front());
                        m_Jobs.pop();
                    }
                    job();
                }
            });
        }
    }

    WorkerPool::~WorkerPool() {
        {
            std::lock_guard lock(m_Mutex);
            m_Stop = true;
        }
        m_Condition.notify_all();
        for (auto &thread : m_Threads) {
            if (thread.joinable()) thread.join();
        }
    }

    void WorkerPool::Enqueue(std::function<void()> job) {
        {
            std::lock_guard lock(m_Mutex);
            m_Jobs.push(std::move(job));
        }
        m_Condition.notify_one();
    }

} // namespace axiom
