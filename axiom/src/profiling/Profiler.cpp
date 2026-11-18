/**
 * @file axiom/src/profiling/Profiler.cpp
 * @brief Implements the profiler module.
 */

#include "axiom/profiling/Profiler.h"
#include "axiom/core/Logger.h"

#include <chrono>
#include <mutex>
#include <thread>

namespace axiom::profiling {

    using Clock = std::chrono::steady_clock;

    static constexpr uint32_t maxFrames = 120;

    /** @brief Stores the data for the thread state. */
    struct ThreadState {
        /** @brief Stores the events. */
        std::vector<CPUEvent> Events;
        /** @brief Stores the event stack. */
        std::vector<uint32_t> EventStack;
        /** @brief Stores the depth. */
        uint32_t Depth = 0;
    };

    static thread_local ThreadState gThreadState;

    /** @brief Stores the data for the profiler state. */
    struct ProfilerState {
        /** @brief Stores the frames. */
        std::array<FrameData, maxFrames> Frames;
        /** @brief Stores the frame index. */
        uint32_t FrameIndex = 0;

        /** @brief Stores the frame start. */
        uint64_t FrameStart = 0;

        /** @brief Stores the mutex. */
        std::mutex Mutex;
    };

    static ProfilerState gState;

    /**
     * @brief Returns the current time.
     * @return The result of the operation.
     */
    static uint64_t now() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
                   Clock::now().time_since_epoch())
            .count();
    }

    double CPUEvent::durationMs() const { return (End - Start) / 1000000.0; }

    /**
     * @brief Returns the thread ID.
     * @return The result of the operation.
     */
    static uint32_t getThreadId() {
        static std::hash<std::thread::id> hasher;
        return (uint32_t)hasher(std::this_thread::get_id());
    }

    void Profiler::printLastFrame() {
        const FrameData &frame = getFrame(0);

        AXIOM_INFO("Frame Time: {} ms", frame.FrameTimeMs);

        for (const auto &event : frame.CPUEvents) {
            std::string indent(event.Depth * 2, ' ');

            AXIOM_INFO("{}{} - {:.3f} ms", indent, event.Name,
                       event.durationMs());
        }
    }

    void Profiler::initialize() {
        static bool initialized = false;

        if (!initialized) {
            AXIOM_INFO("Axiom Profiler initialized");
            initialized = true;
        }
    }

    void Profiler::beginFrame() {
        initialize();

        auto &frame = gState.Frames[gState.FrameIndex];
        frame.CPUEvents.clear();

        gState.FrameStart = now();
    }

    void Profiler::endFrame() {
        uint64_t frameEnd = now();

        auto &frame = gState.Frames[gState.FrameIndex];

        frame.FrameTimeMs = (frameEnd - gState.FrameStart) / 1000000.0;

        {
            std::lock_guard lock(gState.Mutex);

            frame.CPUEvents.insert(frame.CPUEvents.end(),
                                   gThreadState.Events.begin(),
                                   gThreadState.Events.end());
        }

        gThreadState.Events.clear();

        gState.FrameIndex = (gState.FrameIndex + 1) % maxFrames;

        if (!gThreadState.EventStack.empty()) {
            AXIOM_WARN("Profiler: Unclosed profiling scopes detected!");
            gThreadState.EventStack.clear();
        }
    }

    void Profiler::pushEvent(std::string_view name) {
        CPUEvent event{};
        event.Name = name.data();
        event.Start = now();
        event.ThreadID = getThreadId();
        event.Depth = gThreadState.Depth++;

        gThreadState.Events.push_back(event);

        gThreadState.EventStack.push_back(
            static_cast<uint32_t>(gThreadState.Events.size() - 1));
    }

    void Profiler::popEvent() {
        if (gThreadState.EventStack.empty())
            return;

        uint64_t end = now();

        uint32_t index = gThreadState.EventStack.back();
        gThreadState.EventStack.pop_back();

        gThreadState.Events[index].End = end;

        gThreadState.Depth--;
    }

    const FrameData &Profiler::getFrame(uint32_t index) {
        uint32_t i = (gState.FrameIndex + maxFrames - index - 1) % maxFrames;

        return gState.Frames[i];
    }

    CPUScope::CPUScope(std::string_view name) { Profiler::pushEvent(name); }

    CPUScope::~CPUScope() { Profiler::popEvent(); }

} // namespace axiom::profiling