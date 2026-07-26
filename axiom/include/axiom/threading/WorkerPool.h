#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace axiom {

    // Interimsloesung fuer asynchrones Asset-Laden (fester Thread-Pool +
    // Queue). Wird in Phase 11 durch FiberTaskingLib ersetzt, sobald das
    // Fiber-Job-System existiert - AssetManagers oeffentliche API
    // (RequestLoad -> AssetHandle) aendert sich dabei NICHT, nur die interne
    // Ausfuehrung von Jobs.
    class WorkerPool {
      public:
        explicit WorkerPool(size_t threadCount);
        ~WorkerPool();

        WorkerPool(const WorkerPool &) = delete;
        WorkerPool &operator=(const WorkerPool &) = delete;

        void Enqueue(std::function<void()> job);

      private:
        std::vector<std::thread> m_Threads;
        std::queue<std::function<void()>> m_Jobs;
        std::mutex m_Mutex;
        std::condition_variable m_Condition;
        bool m_Stop = false;
    };

} // namespace axiom
