/**
 * @file axiom/include/axiom/threading/WorkerPool.h
 * @brief Declares the public interface for the worker pool.
 */

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
    /**
     * @brief Executes queued jobs on a fixed set of worker threads.
     * @tparam WorkerPool Template type used by this type.
     */
    class WorkerPool {
      public:
        /**
         * @brief Creates an instance of worker pool.
         * @param threadCount The thread count used by this operation.
         */
        explicit WorkerPool(size_t threadCount);
        /**
         * @brief Releases resources owned by WorkerPool.
         */
        ~WorkerPool();

        /**
         * @brief Creates an instance of worker pool.
         * @param WorkerPool The worker pool used by this operation.
         */
        WorkerPool(const WorkerPool &) = delete;
        WorkerPool &operator=(const WorkerPool &) = delete;

        /**
         * @brief Adds work to the queue.
         * @param job The job used by this operation.
         */
        void enqueue(std::function<void()> job);

      private:
        /** @brief Stores the threads. */
        std::vector<std::thread> m_threads;
        std::queue<std::function<void()>> m_jobs;
        /** @brief Stores the mutex. */
        std::mutex m_mutex;
        /** @brief Stores the condition. */
        std::condition_variable m_condition;
        /** @brief Stores the stop. */
        bool m_stop = false;
    };

} // namespace axiom
